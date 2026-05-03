#pragma once

#include "Geo.h"
#include "Types.h"
#include <string>
#include <vector>
#include <functional>

enum class WaypointAction {
    FLY_OVER,
    FLY_TO,
    LOITER,
    RTH
};

struct MissionWaypoint {
    Coordinate coordinate;
    std::string name;
    double altitude = 0.0;
    WaypointAction action = WaypointAction::FLY_OVER;
    double loiterDuration = 0.0;
};

struct MissionCondition {
    std::string expression;
    std::string action;
    std::string target;
};

struct ChecklistItem {
    std::string label;
    bool required = true;
    bool autoCheck = false;
    std::string status = "PENDING";
};

struct Checklist {
    std::string name;
    std::vector<ChecklistItem> items;
    bool allPassed() const;
};

class Mission {
public:
    std::string id;
    std::string droneId;
    std::vector<MissionWaypoint> waypoints;
    Coordinate homeBase;
    int currentWaypointIndex = 0;
    MissionState state = MissionState::IDLE;
    std::vector<MissionCondition> conditions;

    Mission() = default;
    Mission(const std::string& missionId, const std::string& droneIdent);

    void advanceWaypoint();
    void start();
    void abort();
    void returnToHome();
    bool isComplete() const;
    double totalDistance() const;
    double estimatedDuration(double speed) const;
    double estimatedBatteryUse(double batteryDrainRate) const;
    MissionWaypoint getCurrentWaypoint() const;

    MissionWaypoint getHomeAsWaypoint() const;

    void evaluateConditions();
    void addCondition(const MissionCondition& cond);

    void addChecklist(const Checklist& cl);
    const Checklist& getChecklist() const;
    bool checkPreFlight() const;
    void overrideChecklistItem(const std::string& label, const std::string& status);

private:
    Checklist preFlightChecklist;
    bool conditionTriggered = false;
};

class MissionExecutor {
public:
    MissionExecutor() = default;

    void update(Mission& mission, const Coordinate& dronePos, double droneSpeed,
                double batteryLevel, double batteryDrainRate, double deltaTime);

    void uploadMission(Mission& mission, const std::vector<MissionWaypoint>& wps,
                       const Coordinate& home);

    std::function<void(const std::string& droneId, const std::string& event)> onEvent;

    bool isArrived(const Coordinate& dronePos, const Coordinate& waypointPos,
                   double arrivalRadius = 0.1) const;

    void setArrivalRadius(double radius) { arrivalRadius = radius; }

private:
    double arrivalRadius = 0.1;
    double loiterTimer = 0.0;
    bool isLoitering = false;
};
