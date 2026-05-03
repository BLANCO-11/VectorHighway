#include "../include/LinkManager.h"
#include <algorithm>
#include <iostream>

LinkManager::LinkManager(const std::string& id) : droneId(id) {}

void LinkManager::update(double deltaTime, bool heartbeatReceived) {
    if (heartbeatReceived) {
        lastHeartbeat = 0.0;
        lolTimer = 0.0;
        lolActive = false;

        signalStrength = std::min(1.0, signalStrength + deltaTime * 0.1);
        packetLoss = std::max(0.0, packetLoss - deltaTime * 0.05);
    } else {
        lastHeartbeat += deltaTime;
        signalStrength = std::max(0.0, signalStrength - deltaTime * 0.02);
        packetLoss = std::min(1.0, packetLoss + deltaTime * 0.01);

        if (isLinkLost()) {
            lolTimer += deltaTime;
            if (lolTimer > 30.0) {
                lolActive = true;
            }
        }
    }
}

bool LinkManager::shouldFailover() const {
    return signalStrength < 0.2 || packetLoss > 0.5 || latencyMs > 1000.0;
}

void LinkManager::executeFailover() {
    if (backupLinkType != LinkType::NONE) {
        std::swap(linkType, backupLinkType);
        signalStrength = 0.8;
        packetLoss = 0.05;
        std::cout << "[LinkManager] " << droneId << " failed over to "
                  << static_cast<int>(linkType) << std::endl;
    }
}

bool LinkManager::isLinkLost() const {
    return lastHeartbeat > 10.0 || signalStrength < 0.05;
}

void LinkManager::reset() {
    linkType = LinkType::LOS;
    signalStrength = 1.0;
    latencyMs = 0.0;
    packetLoss = 0.0;
    lolActive = false;
    lolTimer = 0.0;
}

void LinkManager::setHeartbeat() {
    lastHeartbeat = 0.0;
}
