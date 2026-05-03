#pragma once

#include <string>

struct CommandEvent {
    std::string type;
    std::string targetId;
    double lat = 0.0;
    double lon = 0.0;
    double radius = 0.0;
    double speed = -1.0;
    double drain = -1.0;
    std::string groupId;
    std::string missionJson;
};

struct ExternalTelemetry {
    std::string id;
    std::string groupId;
    double lat = 0.0;
    double lon = 0.0;
    double alt = 0.0;
    double heading = 0.0;
    double battery = 100.0;
    double speed = 0.0;
    std::string status = "FLYING";
};