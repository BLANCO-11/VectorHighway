#pragma once

#include <string>
#include <vector>
#include <chrono>

using DroneID = std::string;
using GroupID = std::string;

constexpr double EARTH_RADIUS_KM = 6371.0;
constexpr double SIMULATION_TICK_HZ = 20.0;
constexpr double SIMULATION_TICK_S = 1.0 / SIMULATION_TICK_HZ;
constexpr int DEFAULT_PORT = 7200;
constexpr double STALE_DRONE_TIMEOUT_S = 30.0;
constexpr double BATTERY_MAX = 100.0;

enum class MissionState {
    IDLE,
    UPLOADED,
    IN_PROGRESS,
    PAUSED,
    COMPLETED,
    FAILED,
    RTH,
    ABORTED,
    EMERGENCY
};

enum class AlertLevel {
    NOMINAL,
    CAUTION,
    WARNING,
    CRITICAL
};

enum class ConnectionStatus {
    CONNECTED,
    STALE,
    DISCONNECTED,
    IDLE
};

inline const char* missionStateToString(MissionState s) {
    switch (s) {
        case MissionState::IDLE: return "IDLE";
        case MissionState::UPLOADED: return "UPLOADED";
        case MissionState::IN_PROGRESS: return "IN_PROGRESS";
        case MissionState::PAUSED: return "PAUSED";
        case MissionState::COMPLETED: return "COMPLETED";
        case MissionState::FAILED: return "FAILED";
        case MissionState::RTH: return "RTH";
        case MissionState::ABORTED: return "ABORTED";
        case MissionState::EMERGENCY: return "EMERGENCY";
        default: return "UNKNOWN";
    }
}

inline const char* alertLevelToString(AlertLevel l) {
    switch (l) {
        case AlertLevel::NOMINAL: return "NOMINAL";
        case AlertLevel::CAUTION: return "CAUTION";
        case AlertLevel::WARNING: return "WARNING";
        case AlertLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

inline const char* connectionStatusToString(ConnectionStatus s) {
    switch (s) {
        case ConnectionStatus::CONNECTED: return "CONNECTED";
        case ConnectionStatus::STALE: return "STALE";
        case ConnectionStatus::DISCONNECTED: return "DISCONNECTED";
        case ConnectionStatus::IDLE: return "IDLE";
        default: return "UNKNOWN";
    }
}