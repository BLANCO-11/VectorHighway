#pragma once

#include "Types.h"
#include <string>
#include <chrono>
#include <map>

enum class LinkType { LOS, SATCOM, CELLULAR, NONE };

class LinkManager {
public:
    std::string droneId;
    LinkType linkType = LinkType::LOS;
    LinkType backupLinkType = LinkType::NONE;
    double signalStrength = 1.0;
    double latencyMs = 0.0;
    double packetLoss = 0.0;
    double lastHeartbeat = 0.0;
    std::string lolProcedure = "LOITER_AND_RTH";
    double lolTimer = 0.0;
    bool lolActive = false;

    LinkManager() = default;
    explicit LinkManager(const std::string& id);

    void update(double deltaTime, bool heartbeatReceived);
    bool shouldFailover() const;
    void executeFailover();
    bool isLinkLost() const;
    void reset();
    void setHeartbeat();
};
