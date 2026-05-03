#pragma once

#include <cmath>
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static constexpr double EARTH_RADIUS_KM_GEO = 6371.0;

class Vector3 {
public:
    double x, y, z;
    Vector3(double x = 0.0, double y = 0.0, double z = 0.0) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
    Vector3 operator*(double scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }

    double magnitude() const { return std::sqrt(x * x + y * y + z * z); }

    Vector3 normalized() const {
        double mag = magnitude();
        return (mag > 0) ? (*this * (1.0 / mag)) : Vector3(0, 0, 0);
    }
};

class Coordinate {
public:
    double latitude;
    double longitude;
    double altitude;

    Coordinate(double lat = 0.0, double lon = 0.0, double alt = 0.0)
        : latitude(lat), longitude(lon), altitude(alt) {}

    double distanceTo(const Coordinate& other) const {
        constexpr double R = 6371.0;
        double dLat = toRad(other.latitude - latitude);
        double dLon = toRad(other.longitude - longitude);
        double a = std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
                   std::cos(toRad(latitude)) * std::cos(toRad(other.latitude)) *
                   std::sin(dLon / 2.0) * std::sin(dLon / 2.0);
        double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
        return R * c;
    }

    double bearingTo(const Coordinate& other) const {
        double lat1 = toRad(latitude);
        double lat2 = toRad(other.latitude);
        double dLon = toRad(other.longitude - longitude);
        double y = std::sin(dLon) * std::cos(lat2);
        double x = std::cos(lat1) * std::sin(lat2) -
                   std::sin(lat1) * std::cos(lat2) * std::cos(dLon);
        double brng = std::atan2(y, x);
        return std::fmod((toDeg(brng) + 360.0), 360.0);
    }

    Vector3 toECEF() const {
        double latRad = toRad(latitude);
        double lonRad = toRad(longitude);
        double r = EARTH_RADIUS_KM_GEO + altitude;
        return Vector3(
            r * std::cos(latRad) * std::cos(lonRad),
            r * std::cos(latRad) * std::sin(lonRad),
            r * std::sin(latRad)
        );
    }

    static Coordinate fromECEF(const Vector3& ecef) {
        double r = std::sqrt(ecef.x * ecef.x + ecef.y * ecef.y + ecef.z * ecef.z);
        double latRad = std::asin(ecef.z / r);
        double lonRad = std::atan2(ecef.y, ecef.x);
        return Coordinate(toDeg(latRad), toDeg(lonRad), r - EARTH_RADIUS_KM_GEO);
    }

private:
    static double toRad(double degree) { return degree * M_PI / 180.0; }
    static double toDeg(double radian) { return radian * 180.0 / M_PI; }
};

class Waypoint {
public:
    Coordinate coordinate;
    std::string name;
    Waypoint(const Coordinate& coord, const std::string& name = "")
        : coordinate(coord), name(name) {}
};

// ENU Utility Functions

inline Vector3 ecefToEnu(const Vector3& pointEcef, const Vector3& originEcef, const Coordinate& originGeo) {
    double latRad = originGeo.latitude * M_PI / 180.0;
    double lonRad = originGeo.longitude * M_PI / 180.0;
    Vector3 delta = pointEcef - originEcef;
    double east = -std::sin(lonRad) * delta.x + std::cos(lonRad) * delta.y;
    double north = -std::sin(latRad) * std::cos(lonRad) * delta.x
                   - std::sin(latRad) * std::sin(lonRad) * delta.y
                   + std::cos(latRad) * delta.z;
    double up = std::cos(latRad) * std::cos(lonRad) * delta.x
               + std::cos(latRad) * std::sin(lonRad) * delta.y
               + std::sin(latRad) * delta.z;
    return Vector3(east, north, up);
}

inline Coordinate enuToCoordinate(double east, double north, double up, const Coordinate& origin) {
    double latRad = origin.latitude * M_PI / 180.0;
    double lonRad = origin.longitude * M_PI / 180.0;
    double x = -std::sin(lonRad) * east - std::sin(latRad) * std::cos(lonRad) * north + std::cos(latRad) * std::cos(lonRad) * up;
    double y = std::cos(lonRad) * east - std::sin(latRad) * std::sin(lonRad) * north + std::cos(latRad) * std::sin(lonRad) * up;
    double z = std::cos(latRad) * north + std::sin(latRad) * up;
    Vector3 originEcef = origin.toECEF();
    Vector3 pointEcef(originEcef.x + x, originEcef.y + y, originEcef.z + z);
    return Coordinate::fromECEF(pointEcef);
}

inline double crossTrackDistance(const Coordinate& point, const Coordinate& segStart, const Coordinate& segEnd) {
    double distStartToEnd = segStart.distanceTo(segEnd);
    if (distStartToEnd < 1e-10) return point.distanceTo(segStart);
    double bearingStartEnd = segStart.bearingTo(segEnd);
    double bearingStartPoint = segStart.bearingTo(point);
    double angularDist = point.distanceTo(segStart) / EARTH_RADIUS_KM_GEO;
    double crossTrackRad = std::asin(std::sin(angularDist) *
        std::sin((bearingStartPoint - bearingStartEnd) * M_PI / 180.0));
    return crossTrackRad * EARTH_RADIUS_KM_GEO;
}

inline double fractionAlong(const Coordinate& segStart, const Coordinate& segEnd, const Coordinate& point) {
    double distStartToEnd = segStart.distanceTo(segEnd);
    if (distStartToEnd < 1e-10) return 0.0;
    double bearingStartEnd = segStart.bearingTo(segEnd);
    double bearingStartPoint = segStart.bearingTo(point);
    double angularDist = point.distanceTo(segStart) / EARTH_RADIUS_KM_GEO;
    double alongFrac = std::cos(angularDist) / std::cos(crossTrackDistance(point, segStart, segEnd) / EARTH_RADIUS_KM_GEO);
    double alongDist = std::acos(std::max(-1.0, std::min(1.0, alongFrac))) * EARTH_RADIUS_KM_GEO;
    double dot = std::cos((bearingStartPoint - bearingStartEnd) * M_PI / 180.0);
    return (dot >= 0 ? alongDist : -alongDist) / distStartToEnd;
}

struct TangentBypass {
    Coordinate approachLeft, tangentLeft, exitLeft;
    Coordinate approachRight, tangentRight, exitRight;
};

inline TangentBypass computeTangentBypass(const Coordinate& obsCenter, double radiusKm,
                                           const Coordinate& from, const Coordinate& to) {
    TangentBypass result;
    Vector3 fromEcef = from.toECEF();
    Vector3 toEcef = to.toECEF();
    Vector3 obsEcef = obsCenter.toECEF();
    Vector3 fromEnu = ecefToEnu(fromEcef, obsEcef, obsCenter);
    Vector3 toEnu = ecefToEnu(toEcef, obsEcef, obsCenter);
    double dx = toEnu.x - fromEnu.x;
    double dy = toEnu.y - fromEnu.y;
    double angle = std::atan2(dy, dx);
    double perpAngleLeft = angle + M_PI / 2.0;
    double perpAngleRight = angle - M_PI / 2.0;
    double margin = radiusKm + 0.5;
    double approachDist = margin * 1.5;
    double exitDist = margin * 1.5;

    auto tangentPoint = [&](double sideAngle, double) -> Coordinate {
        double tx = margin * std::cos(sideAngle);
        double ty = margin * std::sin(sideAngle);
        return enuToCoordinate(tx, ty, 0.0, obsCenter);
    };

    auto approachPoint = [&](double sideAngle) -> Coordinate {
        double ax = approachDist * std::cos(sideAngle);
        double ay = approachDist * std::sin(sideAngle);
        return enuToCoordinate(ax, ay, 0.0, obsCenter);
    };

    auto exitPoint = [&](double exitAngle) -> Coordinate {
        double ex = exitDist * std::cos(exitAngle);
        double ey = exitDist * std::sin(exitAngle);
        return enuToCoordinate(ex, ey, 0.0, obsCenter);
    };

    result.approachLeft = approachPoint(perpAngleLeft);
    result.tangentLeft = tangentPoint(perpAngleLeft, margin);
    result.exitLeft = exitPoint(perpAngleLeft);
    result.approachRight = approachPoint(perpAngleRight);
    result.tangentRight = tangentPoint(perpAngleRight, margin);
    result.exitRight = exitPoint(perpAngleRight);

    return result;
}
