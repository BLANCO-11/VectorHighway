#pragma once

#include <cmath>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Coordinate {
public:
    double latitude;
    double longitude;
    double altitude;

    Coordinate(double lat = 0.0, double lon = 0.0, double alt = 0.0)
        : latitude(lat), longitude(lon), altitude(alt) {}

    double distanceTo(const Coordinate& other) const {
        constexpr double R = 6371.0; // Earth radius in km
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

private:
    static double toRad(double degree) {
        return degree * M_PI / 180.0;
    }
    static double toDeg(double radian) {
        return radian * 180.0 / M_PI;
    }
};

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

class Waypoint {
public:
    Coordinate coordinate;
    std::string name;
    Waypoint(const Coordinate& coord, const std::string& name = "") 
        : coordinate(coord), name(name) {}
};