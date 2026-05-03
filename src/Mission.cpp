#include "../include/Mission.h"
#include <algorithm>
#include <iostream>

Mission::Mission(const std::string& missionId, const std::string& droneIdent)
    : id(missionId), droneId(droneIdent) {}

void Mission::advanceWaypoint() {
    if (currentWaypointIndex < static_cast<int>(waypoints.size()) - 1) {
        currentWaypointIndex++;
    } else if (currentWaypointIndex >= static_cast<int>(waypoints.size()) - 1) {
        state = MissionState::COMPLETED;
    }
}

void Mission::start() {
    if (state == MissionState::UPLOADED) {
        state = MissionState::IN_PROGRESS;
        currentWaypointIndex = 0;
    }
}

void Mission::abort() {
    state = MissionState::ABORTED;
}

void Mission::returnToHome() {
    state = MissionState::RTH;
}

bool Mission::isComplete() const {
    return state == MissionState::COMPLETED;
}

double Mission::totalDistance() const {
    double dist = 0.0;
    for (size_t i = 1; i < waypoints.size(); ++i) {
        dist += waypoints[i - 1].coordinate.distanceTo(waypoints[i].coordinate);
    }
    if (!waypoints.empty()) {
        dist += waypoints.back().coordinate.distanceTo(homeBase);
    }
    return dist;
}

double Mission::estimatedDuration(double speed) const {
    if (speed <= 0.0) return 0.0;
    return totalDistance() / speed;
}

double Mission::estimatedBatteryUse(double batteryDrainRate) const {
    return estimatedDuration(0.25) * batteryDrainRate;
}

MissionWaypoint Mission::getCurrentWaypoint() const {
    if (currentWaypointIndex >= 0 && currentWaypointIndex < static_cast<int>(waypoints.size())) {
        return waypoints[currentWaypointIndex];
    }
    return getHomeAsWaypoint();
}

MissionWaypoint Mission::getHomeAsWaypoint() const {
    MissionWaypoint wp;
    wp.coordinate = homeBase;
    wp.name = "HOME";
    wp.action = WaypointAction::RTH;
    return wp;
}

void Mission::evaluateConditions() {
    for (const auto& cond : conditions) {
        if (cond.action == "skip" && !conditionTriggered) {
            advanceWaypoint();
            conditionTriggered = true;
        }
    }
}

void Mission::addCondition(const MissionCondition& cond) {
    conditions.push_back(cond);
}

bool Mission::checkPreFlight() const {
    return preFlightChecklist.allPassed();
}

const Checklist& Mission::getChecklist() const {
    return preFlightChecklist;
}

void Mission::overrideChecklistItem(const std::string& label, const std::string& status) {
    for (auto& item : preFlightChecklist.items) {
        if (item.label == label) {
            item.status = status;
            break;
        }
    }
}

void Mission::addChecklist(const Checklist& cl) {
    preFlightChecklist = cl;
}

bool Checklist::allPassed() const {
    for (const auto& item : items) {
        if (item.required && item.status != "PASS") return false;
    }
    return true;
}

void MissionExecutor::update(Mission& mission, const Coordinate& dronePos,
                              double droneSpeed, double batteryLevel,
                              double batteryDrainRate, double deltaTime) {

    if (mission.state != MissionState::IN_PROGRESS && mission.state != MissionState::RTH) return;

    if (mission.state == MissionState::RTH) {
        if (isArrived(dronePos, mission.homeBase)) {
            mission.state = MissionState::COMPLETED;
            if (onEvent) onEvent(mission.droneId, "RTH_COMPLETED");
        }
        return;
    }

    MissionWaypoint current = mission.getCurrentWaypoint();
    if (isArrived(dronePos, current.coordinate)) {
        if (current.action == WaypointAction::LOITER) {
            if (!isLoitering) {
                isLoitering = true;
                loiterTimer = current.loiterDuration;
            }
            loiterTimer -= deltaTime;
            if (loiterTimer <= 0.0) {
                isLoitering = false;
                mission.evaluateConditions();
                mission.advanceWaypoint();
                if (onEvent) onEvent(mission.droneId, "WAYPOINT_REACHED");
            }
        } else {
            mission.advanceWaypoint();
            if (onEvent) onEvent(mission.droneId, "WAYPOINT_REACHED");
        }

        if (mission.isComplete()) {
            if (onEvent) onEvent(mission.droneId, "MISSION_COMPLETE");
        }
    }

    if (batteryLevel < 10.0) {
        mission.state = MissionState::RTH;
        if (onEvent) onEvent(mission.droneId, "RTH_LOW_BATTERY");
    }
}

void MissionExecutor::uploadMission(Mission& mission, const std::vector<MissionWaypoint>& wps,
                                     const Coordinate& home) {
    mission.waypoints = wps;
    mission.homeBase = home;
    mission.currentWaypointIndex = 0;
    mission.state = MissionState::UPLOADED;
}

bool MissionExecutor::isArrived(const Coordinate& dronePos, const Coordinate& waypointPos,
                                 double radius) const {
    return dronePos.distanceTo(waypointPos) <= (radius > 0.0 ? radius : arrivalRadius);
}
