#pragma once

#include "Geo.h"
#include <string>
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Phase 2: Coordinate System Conversion
class CoordinateConversion {
public:
    static constexpr double EARTH_RADIUS_KM = 6371.0;

    // Convert Spherical (Lat, Lon, Alt) to Cartesian (X, Y, Z)
    static Vector3 sphericalToCartesian(const Coordinate& coord) {
        // Assuming Coordinate exposes latitude, longitude, and altitude
        double latRad = coord.latitude * M_PI / 180.0;
        double lonRad = coord.longitude * M_PI / 180.0;
        double r = EARTH_RADIUS_KM + coord.altitude;

        double x = r * cos(latRad) * cos(lonRad);
        double y = r * cos(latRad) * sin(lonRad);
        double z = r * sin(latRad);

        return Vector3(x, y, z);
    }

    // Convert Cartesian (X, Y, Z) to Spherical (Lat, Lon, Alt)
    static Coordinate cartesianToSpherical(const Vector3& vec) {
        double r = std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
        double latRad = std::asin(vec.z / r);
        double lonRad = std::atan2(vec.y, vec.x);

        double latitude = latRad * 180.0 / M_PI;
        double longitude = lonRad * 180.0 / M_PI;
        double altitude = r - EARTH_RADIUS_KM;

        return Coordinate(latitude, longitude, altitude);
    }
};

// Phase 2: Generic Obstacle Base Class
class Obstacle {
public:
    Coordinate position;
    double radius; // Represents the size of the obstacle
    bool isDynamic;

    Obstacle(Coordinate pos, double rad, bool dynamic = false) 
        : position(pos), radius(rad), isDynamic(dynamic) {}
    
    virtual ~Obstacle() = default;
    
    virtual void updatePosition(double deltaTime) = 0;
};

// Phase 2: Dynamic Obstacle
class DynamicObstacle : public Obstacle {
public:
    double speed;
    double heading;

    DynamicObstacle(Coordinate pos, double rad, double spd, double hdg)
        : Obstacle(pos, rad, true), speed(spd), heading(hdg) {}

    void updatePosition(double deltaTime) override {
        // Simple linear movement for demonstration
        double distance = speed * deltaTime;
        double radHeading = heading * M_PI / 180.0;
        
        // Simplified 2D movement on the surface for the placeholder
        position.latitude += (distance * std::cos(radHeading)) / 111.0; // ~111km per degree lat
        position.longitude += (distance * std::sin(radHeading)) / (111.0 * std::cos(position.latitude * M_PI / 180.0));
    }
};

// Phase 2: Static Obstacle
class StaticObstacle : public Obstacle {
public:
    StaticObstacle(Coordinate pos, double rad)
        : Obstacle(pos, rad, false) {}

    void updatePosition(double deltaTime) override {
        // Static obstacles don't move
    }
};

// Phase 2: Charging Station POI
class ChargingStation {
public:
    Coordinate position;
    std::string name;
    double chargingRate; // Rate at which it charges the battery (e.g., % per minute)

    ChargingStation(Coordinate pos, std::string n, double rate)
        : position(pos), name(n), chargingRate(rate) {}
};