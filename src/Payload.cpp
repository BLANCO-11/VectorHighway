#include "../include/Payload.h"
#include <cmath>

Payload::Payload(const std::string& payloadId, const std::string& droneIdent,
                 const std::string& payloadType)
    : id(payloadId), droneId(droneIdent), type(payloadType) {}

void Payload::setGimbal(double pan, double tilt) {
    gimbalPan = std::fmod(pan + 360.0, 360.0);
    gimbalTilt = std::max(-90.0, std::min(90.0, tilt));
}

void Payload::setZoom(double zoomLevel) {
    zoom = std::max(0.1, std::min(100.0, zoomLevel));
}

double Payload::getPowerDraw() const {
    return powerDraw + (zoom - 1.0) * 0.05 + (std::abs(gimbalPan) + std::abs(gimbalTilt)) * 0.01;
}
