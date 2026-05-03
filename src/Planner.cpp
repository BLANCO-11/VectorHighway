#include "../include/Planner.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <iostream>

std::vector<Coordinate> Planner::smoothPath(
    const std::vector<Coordinate>& waypoints,
    const std::vector<std::shared_ptr<Obstacle>>& obstacles,
    int iterations) {

    if (waypoints.size() < 2) return waypoints;

    std::vector<Coordinate> smoothed = waypoints;

    for (int iter = 0; iter < iterations; ++iter) {
        auto repulsions = buildRepulsionField(smoothed, obstacles);
        for (size_t i = 1; i + 1 < smoothed.size(); ++i) {
            smoothed[i].latitude += repulsions[i].x;
            smoothed[i].longitude += repulsions[i].y;
        }
    }

    return smoothed;
}

std::vector<Coordinate> Planner::localReplan(
    const Coordinate& currentPos,
    const Coordinate& targetPos,
    double heading,
    const std::vector<std::shared_ptr<Obstacle>>& obstacles,
    double lookAheadDist) {

    std::vector<Coordinate> result;
    result.push_back(currentPos);

    for (const auto& obs : obstacles) {
        if (segmentIntersectsObstacle(currentPos, targetPos, obs)) {
            auto rrtPath = sampleRRT(currentPos, targetPos, obstacles);
            if (!rrtPath.empty()) {
                result.insert(result.end(), rrtPath.begin() + 1, rrtPath.end());
                return result;
            }
            Coordinate evade(
                currentPos.latitude + (obs->position.latitude - currentPos.latitude) * 0.5,
                currentPos.longitude + (obs->position.longitude - currentPos.longitude) * 0.5,
                0.0
            );
            result.push_back(evade);
        }
    }

    result.push_back(targetPos);
    return result;
}

std::vector<Vector3> Planner::buildRepulsionField(
    const std::vector<Coordinate>& waypoints,
    const std::vector<std::shared_ptr<Obstacle>>& obstacles,
    double repulsionRadius,
    double repulsionStrength) {

    std::vector<Vector3> repulsions(waypoints.size());

    for (size_t i = 0; i < waypoints.size(); ++i) {
        Vector3 totalRepulsion;
        for (const auto& obs : obstacles) {
            double d = waypoints[i].distanceTo(obs->position);
            if (d < repulsionRadius && d > 0.01) {
                double strength = repulsionStrength * (1.0 - d / repulsionRadius) / (d * d);
                totalRepulsion.x += strength * (waypoints[i].latitude - obs->position.latitude);
                totalRepulsion.y += strength * (waypoints[i].longitude - obs->position.longitude);
            }
        }
        repulsions[i] = Vector3(
            totalRepulsion.x * repulsionStrength,
            totalRepulsion.y * repulsionStrength,
            1.0
        );
    }

    return repulsions;
}

bool Planner::segmentIntersectsObstacle(
    const Coordinate& a, const Coordinate& b,
    const std::shared_ptr<Obstacle>& obs) const {

    double distA = a.distanceTo(obs->position);
    double distB = b.distanceTo(obs->position);

    if (distA <= obs->radius || distB <= obs->radius) return true;

    double segLen = a.distanceTo(b);
    if (segLen < 0.001) return false;

    int steps = std::max(5, static_cast<int>(segLen / obs->radius * 2));
    steps = std::min(steps, 50);

    for (int i = 1; i < steps; ++i) {
        double frac = static_cast<double>(i) / steps;
        Coordinate pt = interpolateGreatCircle(a, b, frac);
        if (pt.distanceTo(obs->position) <= obs->radius) return true;
    }

    return false;
}

std::vector<Coordinate> Planner::sampleRRT(
    const Coordinate& start, const Coordinate& goal,
    const std::vector<std::shared_ptr<Obstacle>>& obstacles,
    int maxSamples, double stepSize) {

    struct RRTNode {
        Coordinate coord;
        int parent;
    };

    std::vector<RRTNode> tree;
    tree.push_back({start, -1});

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> latDist(-90.0, 90.0);
    std::uniform_real_distribution<> lonDist(-180.0, 180.0);

    for (int i = 0; i < maxSamples; ++i) {
        Coordinate sample = (i % 10 == 0) ? goal : Coordinate(latDist(gen), lonDist(gen), 0.0);

        size_t nearestIdx = 0;
        double nearestDist = std::numeric_limits<double>::max();
        for (size_t j = 0; j < tree.size(); ++j) {
            double d = tree[j].coord.distanceTo(sample);
            if (d < nearestDist) {
                nearestDist = d;
                nearestIdx = j;
            }
        }

        Coordinate newPos = steer(tree[nearestIdx].coord, sample, stepSize);

        if (obstacleFree(tree[nearestIdx].coord, newPos, obstacles)) {
            tree.push_back({newPos, static_cast<int>(nearestIdx)});
            if (newPos.distanceTo(goal) < stepSize) {
                std::vector<Coordinate> path;
                int idx = static_cast<int>(tree.size()) - 1;
                while (idx >= 0) {
                    path.push_back(tree[idx].coord);
                    idx = tree[idx].parent;
                }
                std::reverse(path.begin(), path.end());
                path.push_back(goal);
                return path;
            }
        }
    }

    return {};
}

Coordinate Planner::interpolateGreatCircle(
    const Coordinate& a, const Coordinate& b, double fraction) const {

    double lat1 = a.latitude * M_PI / 180.0;
    double lon1 = a.longitude * M_PI / 180.0;
    double lat2 = b.latitude * M_PI / 180.0;
    double lon2 = b.longitude * M_PI / 180.0;

    double d = a.distanceTo(b) / 6371.0;
    if (d < 0.001) return a;

    double A = std::sin((1.0 - fraction) * d) / std::sin(d);
    double B = std::sin(fraction * d) / std::sin(d);

    double x = A * std::cos(lat1) * std::cos(lon1) + B * std::cos(lat2) * std::cos(lon2);
    double y = A * std::cos(lat1) * std::sin(lon1) + B * std::cos(lat2) * std::sin(lon2);
    double z = A * std::sin(lat1) + B * std::sin(lat2);

    double latR = std::atan2(z, std::sqrt(x * x + y * y));
    double lonR = std::atan2(y, x);

    return Coordinate(latR * 180.0 / M_PI, lonR * 180.0 / M_PI, 0.0);
}

double Planner::curvatureCost(const std::vector<Coordinate>& waypoints) const {
    if (waypoints.size() < 3) return 0.0;
    double cost = 0.0;
    for (size_t i = 1; i + 1 < waypoints.size(); ++i) {
        double bearing1 = waypoints[i - 1].bearingTo(waypoints[i]);
        double bearing2 = waypoints[i].bearingTo(waypoints[i + 1]);
        double diff = std::abs(std::fmod(bearing2 - bearing1 + 540.0, 360.0) - 180.0);
        cost += diff;
    }
    return cost;
}

double Planner::distance(const Coordinate& a, const Coordinate& b) const {
    return a.distanceTo(b);
}

bool Planner::pointInObstacle(const Coordinate& p, const std::shared_ptr<Obstacle>& obs) const {
    return p.distanceTo(obs->position) <= obs->radius;
}

Coordinate Planner::randomNear(const Coordinate& center, double radius) const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> angleDist(0.0, 2.0 * M_PI);
    std::uniform_real_distribution<> radiusDist(0.0, radius);

    double angle = angleDist(gen);
    double r = radiusDist(gen);
    double lat = center.latitude + r * std::cos(angle);
    double lon = center.longitude + r * std::sin(angle);
    return Coordinate(lat, lon, 0.0);
}

Coordinate Planner::steer(const Coordinate& from, const Coordinate& to, double stepSize) const {
    double d = from.distanceTo(to);
    if (d <= stepSize) return to;
    double fraction = stepSize / (d + 1e-10);
    return interpolateGreatCircle(from, to, fraction);
}

bool Planner::obstacleFree(const Coordinate& from, const Coordinate& to,
                            const std::vector<std::shared_ptr<Obstacle>>& obstacles) const {
    for (const auto& obs : obstacles) {
        if (segmentIntersectsObstacle(from, to, obs)) return false;
    }
    return true;
}
