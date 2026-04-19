#pragma once

#include "Geo.h"
#include "Environment.h"
#include <iostream>

// Phase 4: UAV Agent with Kinematics and Battery
class UAV {
public:
    Coordinate position;
    double heading;          // Current heading in degrees
    double speed;            // Speed in km/s
    double turnRate;         // Turn rate in degrees/s
    double batteryLevel;     // Battery percentage (100.0 to 0.0)
    double batteryDrainRate; // Base drain rate % per second

    UAV(Coordinate startPos, double startHeading, double uavSpeed, double uavTurnRate)
        : position(startPos),
          heading(startHeading),
          speed(uavSpeed),
          turnRate(uavTurnRate),
          batteryLevel(100.0),
          batteryDrainRate(0.01) // Reduced base drain for better simulation stability
    {
    }

    // Adjusts the UAV's heading towards a target, respecting its turn rate
    void adjustHeading(double targetHeading, double deltaTime) {
        double diff = std::fmod(targetHeading - heading + 360.0, 360.0);
        if (diff > 180.0) {
            diff -= 360.0; // Find shortest turn direction (-180 to 180)
        }

        double maxTurn = turnRate * deltaTime;
        if (std::abs(diff) <= maxTurn) {
            heading = targetHeading;
        } else {
            heading += (diff > 0 ? 1 : -1) * maxTurn;
        }
        heading = std::fmod(heading + 360.0, 360.0);
    }

    // Updates the UAV's position and battery based on delta time
    void update(double deltaTime, double altitudeChange = 0.0) {
        // 1. Calculate battery drain based on movement and altitude changes
        // Drain increases with speed and vertical movement
        double movementFactor = 1.0 + (speed * 0.5); 
        double altitudeFactor = 1.0 + (std::abs(altitudeChange) * 2.0);
        
        // Realism: Hovering consumes battery over time even if speed is 0
        // We'll treat speed as a proxy for energy consumption
        double consumptionRate = batteryDrainRate * movementFactor * altitudeFactor;
        
        double totalDrain = consumptionRate * deltaTime;
        batteryLevel -= totalDrain;
        if (batteryLevel < 0.0) batteryLevel = 0.0;

        // 2. Update altitude
        position.altitude += altitudeChange;

        // 3. Calculate new position based on heading and speed (kinematics)
        double distance = speed * deltaTime;
        double headingRad = heading * M_PI / 180.0;
        double latRad = position.latitude * M_PI / 180.0;

        double newLatRad = std::asin(std::sin(latRad) * std::cos(distance / CoordinateConversion::EARTH_RADIUS_KM) +
                                     std::cos(latRad) * std::sin(distance / CoordinateConversion::EARTH_RADIUS_KM) * std::cos(headingRad));

        double newLonRad = (position.longitude * M_PI / 180.0) + std::atan2(std::sin(headingRad) * std::sin(distance / CoordinateConversion::EARTH_RADIUS_KM) * std::cos(latRad),
                                                                         std::cos(distance / CoordinateConversion::EARTH_RADIUS_KM) - std::sin(latRad) * std::sin(newLatRad));

        position.latitude = newLatRad * 180.0 / M_PI;
        position.longitude = newLonRad * 180.0 / M_PI;
    }
};