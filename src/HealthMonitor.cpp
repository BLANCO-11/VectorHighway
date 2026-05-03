#include "../include/HealthMonitor.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <chrono>

HealthStatus HealthMonitor::check(
    double engineTickHz,
    int wsConnectionCount,
    double wsLastMessageTime,
    const std::map<std::string, DroneHealth>& droneHealthMap) {

    std::lock_guard<std::mutex> lock(mtx);
    HealthStatus status;
    status.engineTickHz = engineTickHz;
    status.wsConnectionCount = wsConnectionCount;
    status.wsLastMessageTime = wsLastMessageTime;
    status.drones = droneHealthMap;
    status.alerts = alerts;

    AlertLevel engineLevel = assessEngineHealth(engineTickHz);
    if (engineLevel != AlertLevel::NOMINAL) {
        double now = static_cast<double>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        if (shouldEmitAlert("engine", engineLevel, now)) {
            std::string msg = engineTickHz < 10.0
                ? "Engine tick rate critically low: " + std::to_string(engineTickHz) + " Hz"
                : "Engine tick rate degraded: " + std::to_string(engineTickHz) + " Hz";
            addAlert(engineLevel, "engine", msg);
        }
    }

    for (const auto& [id, dh] : droneHealthMap) {
        AlertLevel droneLevel = assessDroneHealth(dh);
        if (droneLevel != AlertLevel::NOMINAL) {
            double now = static_cast<double>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count());
            if (shouldEmitAlert(id, droneLevel, now)) {
                std::string msg;
                switch (droneLevel) {
                    case AlertLevel::CRITICAL:
                        msg = "Drone " + id + " in critical state";
                        break;
                    case AlertLevel::WARNING:
                        msg = "Drone " + id + " battery low or stale";
                        break;
                    default:
                        msg = "Drone " + id + " needs attention";
                        break;
                }
                addAlert(droneLevel, id, msg);
            }
        }
    }

    status.alerts = alerts;
    tickCounter = 0;
    return status;
}

void HealthMonitor::addAlert(AlertLevel level, const std::string& source, const std::string& message) {
    Alert alert;
    alert.level = level;
    alert.source = source;
    alert.message = message;
    alert.timestamp = static_cast<double>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
    alerts.push_back(alert);

    if (onAlert) onAlert(alert);

    if (alerts.size() > 100) {
        alerts.erase(alerts.begin());
    }
}

void HealthMonitor::clearAlerts() {
    alerts.clear();
}

std::vector<Alert> HealthMonitor::getAlerts() const {
    return alerts;
}

AlertLevel HealthMonitor::assessDroneHealth(const DroneHealth& dh) const {
    if (dh.connectionStatus == ConnectionStatus::DISCONNECTED) return AlertLevel::CRITICAL;
    if (dh.batteryLevel < 10.0) return AlertLevel::CRITICAL;
    if (dh.batteryLevel < 25.0) return AlertLevel::WARNING;
    if (dh.connectionStatus == ConnectionStatus::STALE) return AlertLevel::WARNING;
    if (dh.batteryLevel < 50.0) return AlertLevel::CAUTION;
    return AlertLevel::NOMINAL;
}

AlertLevel HealthMonitor::assessEngineHealth(double tickHz) const {
    if (tickHz < 10.0) return AlertLevel::CRITICAL;
    if (tickHz < 15.0) return AlertLevel::WARNING;
    return AlertLevel::NOMINAL;
}

bool HealthMonitor::shouldEmitAlert(const std::string& source, AlertLevel level, double now) {
    if (level == AlertLevel::NOMINAL) return false;
    auto it = lastAlertTimes.find(source);
    if (it != lastAlertTimes.end() && (now - it->second) < alertCooldownSeconds) {
        return false;
    }
    lastAlertTimes[source] = now;
    return true;
}

void EmergencyEngine::addRule(const EmergencyRule& rule) {
    std::lock_guard<std::mutex> lock(mtx);
    rules.push_back(rule);
    std::sort(rules.begin(), rules.end(),
              [](const EmergencyRule& a, const EmergencyRule& b) {
                  return a.priority < b.priority;
              });
}

void EmergencyEngine::removeRule(const std::string& ruleId) {
    std::lock_guard<std::mutex> lock(mtx);
    rules.erase(std::remove_if(rules.begin(), rules.end(),
                [&](const EmergencyRule& r) { return r.id == ruleId; }),
                rules.end());
}

void EmergencyEngine::setRules(const std::vector<EmergencyRule>& newRules) {
    std::lock_guard<std::mutex> lock(mtx);
    rules = newRules;
}

const std::vector<EmergencyRule>& EmergencyEngine::getRules() const {
    return rules;
}

void EmergencyEngine::enableRule(const std::string& ruleId, bool enabled) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& r : rules) {
        if (r.id == ruleId) {
            r.enabled = enabled;
            break;
        }
    }
}

std::vector<EmergencyEngine::RuleResult> EmergencyEngine::evaluate(
    double tickHz, int wsCount,
    const std::map<std::string, DroneHealth>& drones,
    double currentTime) {

    std::lock_guard<std::mutex> lock(mtx);
    std::vector<RuleResult> results;

    for (const auto& rule : rules) {
        if (!rule.enabled) continue;

        auto it = lastTriggerTimes.find(rule.id);
        if (it != lastTriggerTimes.end() &&
            (currentTime - it->second) < triggerCooldownSeconds) {
            continue;
        }

        bool triggered = false;
        std::string message;

        for (const auto& [id, dh] : drones) {
            if (evaluateCondition(rule.condition, tickHz, &dh, dh.batteryLevel)) {
                triggered = true;
                message = "Rule '" + rule.name + "' triggered for " + id;
                break;
            }
        }

        if (!triggered) {
            if (evaluateCondition(rule.condition, tickHz, nullptr, 0.0)) {
                triggered = true;
                message = "Rule '" + rule.name + "' triggered (system-level)";
            }
        }

        if (triggered) {
            lastTriggerTimes[rule.id] = currentTime;
            results.push_back({rule.id, rule.action, true, message});
            if (onAction) onAction(rule.action, message);
        }
    }

    return results;
}

bool EmergencyEngine::evaluateCondition(const std::string& condition, double tickHz,
                                         const DroneHealth* drone, double battery) const {
    if (condition.find("battery") != std::string::npos && drone) {
        if (condition.find("< 20") != std::string::npos && battery < 20.0) return true;
        if (condition.find("< 10") != std::string::npos && battery < 10.0) return true;
        if (condition.find("< 5") != std::string::npos && battery < 5.0) return true;
    }
    if (condition.find("tick") != std::string::npos && tickHz < 5.0) return true;
    if (condition.find("disconnect") != std::string::npos && drone &&
        drone->connectionStatus == ConnectionStatus::DISCONNECTED) return true;
    if (condition.find("stale") != std::string::npos && drone &&
        drone->connectionStatus == ConnectionStatus::STALE) return true;
    return false;
}
