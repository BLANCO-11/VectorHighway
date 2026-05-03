#include "../include/GeoUtils.h"
#include <cmath>

double toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

double toDegrees(double radians) {
    return radians * 180.0 / M_PI;
}

bool pointInPolygon(const Coordinate& point, const std::vector<Coordinate>& polygon) {
    if (polygon.size() < 3) return false;
    bool inside = false;
    size_t j = polygon.size() - 1;
    for (size_t i = 0; i < polygon.size(); ++i) {
        if ((polygon[i].longitude > point.longitude) != (polygon[j].longitude > point.longitude) &&
            point.latitude < (polygon[j].latitude - polygon[i].latitude) *
            (point.longitude - polygon[i].longitude) /
            (polygon[j].longitude - polygon[i].longitude) + polygon[i].latitude) {
            inside = !inside;
        }
        j = i;
    }
    return inside;
}

bool pointInCircle(const Coordinate& point, const Coordinate& center, double radiusKm) {
    return point.distanceTo(center) <= radiusKm;
}

double polygonArea(const std::vector<Coordinate>& polygon) {
    if (polygon.size() < 3) return 0.0;
    double area = 0.0;
    size_t j = polygon.size() - 1;
    for (size_t i = 0; i < polygon.size(); ++i) {
        area += (polygon[j].longitude + polygon[i].longitude) *
                (polygon[j].latitude - polygon[i].latitude);
        j = i;
    }
    return std::abs(area) / 2.0;
}

double haversineDistance(double lat1, double lon1, double lat2, double lon2) {
    double dLat = toRadians(lat2 - lat1);
    double dLon = toRadians(lon2 - lon1);
    double a = std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
               std::cos(toRadians(lat1)) * std::cos(toRadians(lat2)) *
               std::sin(dLon / 2.0) * std::sin(dLon / 2.0);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return 6371.0 * c;
}

double crossTrackDistance(const Coordinate& point, const Coordinate& segmentStart,
                           const Coordinate& segmentEnd) {
    double d13 = point.distanceTo(segmentStart) / 6371.0;
    double theta13 = toRadians(segmentStart.bearingTo(point));
    double theta12 = toRadians(segmentStart.bearingTo(segmentEnd));
    return std::asin(std::sin(d13) * std::sin(theta13 - theta12)) * 6371.0;
}

Coordinate midpoint(const Coordinate& a, const Coordinate& b) {
    return Coordinate(
        (a.latitude + b.latitude) / 2.0,
        (a.longitude + b.longitude) / 2.0,
        (a.altitude + b.altitude) / 2.0
    );
}

double elevationAt(const Coordinate& point) {
    return 0.0;
}

double interpolateBearing(double bearing1, double bearing2, double fraction) {
    double diff = std::fmod(bearing2 - bearing1 + 540.0, 360.0) - 180.0;
    return std::fmod(bearing1 + fraction * diff + 360.0, 360.0);
}

bool segmentsIntersect(const Coordinate& a1, const Coordinate& a2,
                        const Coordinate& b1, const Coordinate& b2) {
    auto cross = [](double vx, double vy, double wx, double wy) {
        return vx * wy - vy * wx;
    };

    double d1x = a2.latitude - a1.latitude;
    double d1y = a2.longitude - a1.longitude;
    double d2x = b2.latitude - b1.latitude;
    double d2y = b2.longitude - b1.longitude;

    double denom = cross(d1x, d1y, d2x, d2y);
    if (std::abs(denom) < 1e-10) return false;

    double t = cross(b1.latitude - a1.latitude, b1.longitude - a1.longitude, d2x, d2y) / denom;
    double u = cross(b1.latitude - a1.latitude, b1.longitude - a1.longitude, d1x, d1y) / denom;

    return t >= 0.0 && t <= 1.0 && u >= 0.0 && u <= 1.0;
}
