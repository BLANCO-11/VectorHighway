#pragma once

#include "CommandEvent.h"
#include "Types.h"
#include <mutex>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>

struct ExternalDroneState {
    ExternalTelemetry telemetry;
    std::chrono::steady_clock::time_point lastSeen;
};

class SimulationContext {
public:
    std::vector<CommandEvent> commandQueue;
    std::vector<ExternalTelemetry> externalTelemetry;
    std::unordered_map<std::string, ExternalDroneState> externalDrones;
    std::mutex mtx;

    void pushCommand(const CommandEvent& cmd) {
        std::lock_guard<std::mutex> lock(mtx);
        commandQueue.push_back(cmd);
    }

    std::vector<CommandEvent> drainCommands() {
        std::lock_guard<std::mutex> lock(mtx);
        auto cmds = std::move(commandQueue);
        commandQueue.clear();
        return cmds;
    }

    void pushExternalTelemetry(const ExternalTelemetry& ext) {
        std::lock_guard<std::mutex> lock(mtx);
        externalTelemetry.push_back(ext);
    }

    std::vector<ExternalTelemetry> drainExternalTelemetry() {
        std::lock_guard<std::mutex> lock(mtx);
        auto telemetry = std::move(externalTelemetry);
        externalTelemetry.clear();
        return telemetry;
    }

    void updateExternalDrone(const ExternalTelemetry& ext) {
        std::lock_guard<std::mutex> lock(mtx);
        externalDrones[ext.id] = {ext, std::chrono::steady_clock::now()};
    }

    std::vector<std::string> getStaleDrones() {
        std::lock_guard<std::mutex> lock(mtx);
        std::vector<std::string> stale;
        auto now = std::chrono::steady_clock::now();
        for (auto it = externalDrones.begin(); it != externalDrones.end(); ) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.lastSeen).count();
            if (elapsed >= static_cast<long long>(STALE_DRONE_TIMEOUT_S)) {
                stale.push_back(it->first);
                it = externalDrones.erase(it);
            } else {
                ++it;
            }
        }
        return stale;
    }

    std::vector<ExternalDroneState> getActiveExternalDrones() {
        std::lock_guard<std::mutex> lock(mtx);
        std::vector<ExternalDroneState> active;
        for (const auto& pair : externalDrones) {
            active.push_back(pair.second);
        }
        return active;
    }
};