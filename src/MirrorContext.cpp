#include "../include/MirrorContext.h"
#include <algorithm>
#include <iostream>
#include <sstream>

bool MirrorContext::cloneState(const std::map<std::string, DroneContext>& liveSwarm,
                                const std::vector<Mission>& liveMissions) {
    drones.clear();
    missions.clear();
    conflicts.clear();

    for (const auto& [id, ctx] : liveSwarm) {
        UAV cloned(*ctx.uav);
        drones[id] = std::move(cloned);
    }

    for (const auto& m : liveMissions) {
        missions[m.id] = m;
    }

    createdAt = static_cast<double>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());

    isSimulating = false;
    return !drones.empty();
}

void MirrorContext::simulateStep(double deltaTime, double tickHz) {
    isSimulating = true;
    for (auto& [id, uav] : drones) {
        uav.update(deltaTime, 0.0);
    }
}

bool MirrorContext::promoteState(std::map<std::string, DroneContext>& liveSwarm,
                                  std::vector<Mission>& liveMissions) {
    conflicts = detectConflicts();
    if (!conflicts.empty()) return false;

    for (auto& [id, uav] : drones) {
        auto it = liveSwarm.find(id);
        if (it != liveSwarm.end()) {
            it->second.uav->position = uav.position;
            it->second.uav->heading = uav.heading;
            it->second.uav->currentSpeed = uav.currentSpeed;
            it->second.uav->batteryLevel = uav.batteryLevel;
        }
    }

    for (auto& [id, mission] : missions) {
        auto it = std::find_if(liveMissions.begin(), liveMissions.end(),
                                [&](const Mission& m) { return m.id == id; });
        if (it != liveMissions.end()) {
            *it = mission;
        }
    }

    return true;
}

std::vector<std::string> MirrorContext::detectConflicts() const {
    std::vector<std::string> issues;

    for (const auto& [id, uav] : drones) {
        if (uav.batteryLevel < 10.0) {
            issues.push_back("Drone " + id + " battery critically low (" +
                             std::to_string(uav.batteryLevel) + "%)");
        }
        if (uav.missionState == MissionState::FAILED) {
            issues.push_back("Drone " + id + " mission failed in sandbox");
        }
    }

    return issues;
}

void MirrorContext::reset() {
    drones.clear();
    missions.clear();
    conflicts.clear();
    isSimulating = false;
}

bool MirrorManager::createMirror(const std::map<std::string, DroneContext>& swarm) {
    auto mirror = std::make_unique<MirrorContext>();
    std::vector<Mission> emptyMissions;
    if (!mirror->cloneState(swarm, emptyMissions)) return false;

    activeMirror = std::move(mirror);
    return true;
}

std::vector<std::string> MirrorManager::simulate(double deltaTime, int ticks) {
    if (!activeMirror) return {};
    for (int i = 0; i < ticks; ++i) {
        activeMirror->simulateStep(deltaTime, 60.0);
    }
    return activeMirror->detectConflicts();
}

std::vector<std::string> MirrorManager::promote() {
    if (!activeMirror) return {"No active mirror"};
    std::map<std::string, DroneContext> emptySwarm;
    std::vector<Mission> emptyMissions;
    auto conflicts = activeMirror->detectConflicts();
    if (!conflicts.empty()) return conflicts;

    mirrorHistory.push_back(std::move(*activeMirror));
    activeMirror.reset();
    return {};
}

void MirrorManager::discard() {
    activeMirror.reset();
}

bool MirrorManager::hasActiveMirror() const {
    return activeMirror != nullptr;
}
