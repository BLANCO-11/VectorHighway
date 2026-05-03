#pragma once

#include "Geo.h"
#include "Environment.h"
#include "Types.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <limits>

class Planner {
public:
    struct ObstacleRepulsion {
        Coordinate waypoint;
        Vector3 repulsionVector;
        double weight;
    };

    Planner() = default;

    std::vector<Coordinate> smoothPath(
        const std::vector<Coordinate>& waypoints,
        const std::vector<std::shared_ptr<Obstacle>>& obstacles,
        int iterations = 10
    );

    std::vector<Coordinate> localReplan(
        const Coordinate& currentPos,
        const Coordinate& targetPos,
        double heading,
        const std::vector<std::shared_ptr<Obstacle>>& obstacles,
        double lookAheadDist = 50.0
    );

    std::vector<Vector3> buildRepulsionField(
        const std::vector<Coordinate>& waypoints,
        const std::vector<std::shared_ptr<Obstacle>>& obstacles,
        double repulsionRadius = 10.0,
        double repulsionStrength = 5.0
    );

    bool segmentIntersectsObstacle(
        const Coordinate& a,
        const Coordinate& b,
        const std::shared_ptr<Obstacle>& obs
    ) const;

    std::vector<Coordinate> sampleRRT(
        const Coordinate& start,
        const Coordinate& goal,
        const std::vector<std::shared_ptr<Obstacle>>& obstacles,
        int maxSamples = 100,
        double stepSize = 2.0
    );

    Coordinate interpolateGreatCircle(
        const Coordinate& a,
        const Coordinate& b,
        double fraction
    ) const;

private:
    double curvatureCost(const std::vector<Coordinate>& waypoints) const;
    double distance(const Coordinate& a, const Coordinate& b) const;
    bool pointInObstacle(const Coordinate& p, const std::shared_ptr<Obstacle>& obs) const;
    Coordinate randomNear(const Coordinate& center, double radius) const;
    Coordinate steer(const Coordinate& from, const Coordinate& to, double stepSize) const;
    bool obstacleFree(const Coordinate& from, const Coordinate& to,
                      const std::vector<std::shared_ptr<Obstacle>>& obstacles) const;
};
