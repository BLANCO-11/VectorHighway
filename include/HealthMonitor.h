#pragma once

#include "Types.h"
#include "UAV.h"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <memory>
#include <mutex>
struct Alert {
    AlertLevel level = AlertLevel::NOMINAL;
    std::string source;
    std::string message;
    double timestamp = 0.0;
};

struct DroneHealth {
    double batteryLevel = 100.0;
    double lastTelemetryTime = 0.0;
    std::string missionState = "IDLE";
    ConnectionStatus connectionStatus = ConnectionStatus::CONNECTED;
};

struct HealthStatus {
    double engineTickHz = 20.0;
    int wsConnectionCount = 0;
    double wsLastMessageTime = 0.0;
    std::map<std::string, DroneHealth> drones;
    std::vector<Alert> alerts;
};

struct EmergencyRule {
    std::string id;
    std::string name;
    std::string condition;
    std::string action;
    int priority = 0;
    bool enabled = true;
};

class HealthMonitor {
public:
    HealthMonitor() = default;

    HealthStatus check(
        double engineTickHz,
        int wsConnectionCount,
        double wsLastMessageTime,
        const std::map<std::string, DroneHealth>& droneHealthMap
    );

    void addAlert(AlertLevel level, const std::string& source, const std::string& message);

    void clearAlerts();
    std::vector<Alert> getAlerts() const;

    int getTickCounter() const { return tickCounter; }
    void resetTickCounter() { tickCounter = 0; }
    void incrementTickCounter() { tickCounter++; }

    std::function<void(const Alert&)> onAlert;

    void setAlertCooldown(double seconds) { alertCooldownSeconds = seconds; }

private:
    int tickCounter = 0;
    double alertCooldownSeconds = 5.0;
    std::vector<Alert> alerts;
    std::map<std::string, double> lastAlertTimes;
    mutable std::mutex mtx;

    AlertLevel assessDroneHealth(const DroneHealth& dh) const;
    AlertLevel assessEngineHealth(double tickHz) const;
    bool shouldEmitAlert(const std::string& source, AlertLevel level, double now);
};

class EmergencyEngine {
public:
    EmergencyEngine() = default;

    void addRule(const EmergencyRule& rule);
    void removeRule(const std::string& ruleId);
    void setRules(const std::vector<EmergencyRule>& rules);
    const std::vector<EmergencyRule>& getRules() const;

    struct RuleResult {
        std::string ruleId;
        std::string action;
        bool triggered;
        std::string message;
    };

    std::vector<RuleResult> evaluate(double tickHz, int wsCount,
                                      const std::map<std::string, DroneHealth>& drones,
                                      double currentTime);

    std::function<void(const std::string& action, const std::string& message)> onAction;

    void enableRule(const std::string& ruleId, bool enabled);

private:
    std::vector<EmergencyRule> rules;
    std::map<std::string, double> lastTriggerTimes;
    mutable std::mutex mtx;

    bool evaluateCondition(const std::string& condition, double tickHz,
                           const DroneHealth* drone, double battery) const;
    double triggerCooldownSeconds = 10.0;
};
