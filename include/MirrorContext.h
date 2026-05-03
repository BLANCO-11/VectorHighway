#pragma once

#include "Types.h"
#include "UAV.h"
#include "Mission.h"
#include "SimulationContext.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

class MirrorContext {
public:
    std::string stateId;
    std::string parentStateId;
    std::map<std::string, UAV> drones;
    std::map<std::string, Mission> missions;
    std::vector<std::string> conflicts;
    double createdAt = 0.0;
    bool isSimulating = false;

    MirrorContext() = default;

    bool cloneState(const std::map<std::string, DroneContext>& liveSwarm,
                    const std::vector<Mission>& liveMissions);
    void simulateStep(double deltaTime, double tickHz);
    bool promoteState(std::map<std::string, DroneContext>& liveSwarm,
                      std::vector<Mission>& liveMissions);
    std::vector<std::string> detectConflicts() const;
    void reset();
};

struct MirrorManager {
    std::unique_ptr<MirrorContext> activeMirror;
    std::vector<MirrorContext> mirrorHistory;

    bool createMirror(const std::map<std::string, DroneContext>& swarm);
    std::vector<std::string> simulate(double deltaTime, int ticks);
    std::vector<std::string> promote();
    void discard();
    bool hasActiveMirror() const;
};
