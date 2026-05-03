#pragma once

#include "Types.h"
#include <string>

class Payload {
public:
    std::string id;
    std::string droneId;
    std::string type = "EO_IR";
    double weight = 0.0;
    double powerDraw = 0.0;
    double gimbalPan = 0.0;
    double gimbalTilt = 0.0;
    double zoom = 1.0;

    Payload() = default;
    Payload(const std::string& payloadId, const std::string& droneIdent,
            const std::string& payloadType = "EO_IR");

    void setGimbal(double pan, double tilt);
    void setZoom(double zoomLevel);
    double getPowerDraw() const;
};
