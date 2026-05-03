#include "../include/PathSmoother.h"
#include "../include/Pathfinder.h"
#include "../include/AnchorPointGraph.h"
#include <vector>
#include <algorithm>
#include <random>
#include <limits>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Coordinate PathSmoother::catmullRomPoint(
    const Coordinate& p0, const Coordinate& p1,
    const Coordinate& p2, const Coordinate& p3,
    double t)
{
    double t2 = t * t;
    double t3 = t2 * t;

    double lat = 0.5 * ((2.0 * p1.latitude) +
        (-p0.latitude + p2.latitude) * t +
        (2.0 * p0.latitude - 5.0 * p1.latitude + 4.0 * p2.latitude - p3.latitude) * t2 +
        (-p0.latitude + 3.0 * p1.latitude - 3.0 * p2.latitude + p3.latitude) * t3);

    double lon = 0.5 * ((2.0 * p1.longitude) +
        (-p0.longitude + p2.longitude) * t +
        (2.0 * p0.longitude - 5.0 * p1.longitude + 4.0 * p2.longitude - p3.longitude) * t2 +
        (-p0.longitude + 3.0 * p1.longitude - 3.0 * p2.longitude + p3.longitude) * t3);

    return Coordinate(lat, lon, p1.altitude);
}

std::vector<Coordinate> PathSmoother::catmullRomInterpolate(
    const std::vector<Coordinate>& controlPoints,
    int subdivisions)
{
    if (controlPoints.size() < 2) return controlPoints;
    if (controlPoints.size() == 2) {
        std::vector<Coordinate> result;
        for (int i = 0; i <= subdivisions; ++i) {
            double t = static_cast<double>(i) / subdivisions;
            result.push_back(Coordinate(
                controlPoints[0].latitude + (controlPoints[1].latitude - controlPoints[0].latitude) * t,
                controlPoints[0].longitude + (controlPoints[1].longitude - controlPoints[0].longitude) * t,
                controlPoints[0].altitude
            ));
        }
        return result;
    }

    std::vector<Coordinate> result;
    for (size_t i = 0; i < controlPoints.size() - 1; ++i) {
        const Coordinate& p0 = (i == 0) ? controlPoints[i] : controlPoints[i - 1];
        const Coordinate& p1 = controlPoints[i];
        const Coordinate& p2 = controlPoints[i + 1];
        const Coordinate& p3 = (i + 2 < controlPoints.size()) ? controlPoints[i + 2] : controlPoints[i + 1];

        for (int j = 0; j < subdivisions; ++j) {
            double t = static_cast<double>(j) / subdivisions;
            result.push_back(catmullRomPoint(p0, p1, p2, p3, t));
        }
    }
    result.push_back(controlPoints.back());

    return result;
}

// ObstacleAvoidancePathfinder implementation

std::vector<ObstacleAvoidancePathfinder::Blocker> ObstacleAvoidancePathfinder::findBlockingObstacles(
    const Coordinate& start, const Coordinate& goal,
    const std::vector<Obstacle*>& obstacles,
    const std::vector<NoFlyZone*>& zones,
    double safetyMarginKm)
{
    std::vector<Blocker> blockers;

    for (auto* obs : obstacles) {
        double crossTrack = crossTrackDistance(obs->position, start, goal);
        double frac = fractionAlong(start, goal, obs->position);

        if (frac >= -0.1 && frac <= 1.1 &&
            std::abs(crossTrack) < obs->radius + safetyMarginKm + 1.0) {
            blockers.push_back({obs, nullptr, crossTrack, std::max(0.0, std::min(1.0, frac))});
        }
    }

    for (auto* zone : zones) {
        if (!zone || !zone->active) continue;
        if (zone->shape == NoFlyZoneShape::CIRCLE) {
            double crossTrack = crossTrackDistance(zone->center, start, goal);
            double frac = fractionAlong(start, goal, zone->center);
            if (frac >= -0.1 && frac <= 1.1 &&
                std::abs(crossTrack) < zone->radius + safetyMarginKm + 1.0) {
                Blocker b;
                b.zone = zone;
                b.crossTrackDist = crossTrack;
                b.fractionAlong = std::max(0.0, std::min(1.0, frac));
                blockers.push_back(b);
            }
        }
    }

    std::sort(blockers.begin(), blockers.end(),
        [](const Blocker& a, const Blocker& b) {
            return a.fractionAlong < b.fractionAlong;
        });

    return blockers;
}

std::vector<AnchorPointGraph::AnchorNode> ObstacleAvoidancePathfinder::generateAnchorGraph(
    const Coordinate& start, const Coordinate& goal,
    const std::vector<Blocker>& blockers)
{
    AnchorPointGraph graph;

    int startId = graph.addNode(start, "start", "");
    std::vector<std::vector<int>> layerNodeIds;
    layerNodeIds.push_back({startId});

    for (const auto& blocker : blockers) {
        Coordinate center = blocker.obs ? blocker.obs->position : blocker.zone->center;
        double radius = blocker.obs ? blocker.obs->radius : blocker.zone->radius;

        TangentBypass bypass = computeTangentBypass(center, radius + config_.safetyMarginKm, start, goal);

        std::vector<int> layer;
        layer.push_back(graph.addNode(bypass.approachLeft, std::to_string(blocker.fractionAlong), "left"));
        layer.push_back(graph.addNode(bypass.approachRight, std::to_string(blocker.fractionAlong), "right"));
        layerNodeIds.push_back(layer);
    }

    int goalId = graph.addNode(goal, "goal", "");
    layerNodeIds.push_back({goalId});

    for (size_t i = 1; i < layerNodeIds.size(); ++i) {
        auto& prevLayer = layerNodeIds[i - 1];
        auto& currLayer = layerNodeIds[i];

        int prevStart = prevLayer.front();
        int prevEnd = prevLayer.back();
        int currStart = currLayer.front();
        int currEnd = currLayer.back();

        graph.connectLayer(prevStart, prevEnd, currStart, currEnd, {}, config_.safetyMarginKm);
    }

    auto pathCoords = graph.findPath(startId, goalId);
    if (pathCoords.empty()) return {};

    std::vector<AnchorPointGraph::AnchorNode> result;
    for (auto& node : graph.nodes()) {
        result.push_back(node);
    }
    return result;
}

std::vector<Coordinate> ObstacleAvoidancePathfinder::catmullRomSmooth(
    const std::vector<Coordinate>& ctrlPoints, int subdivisions)
{
    return PathSmoother::catmullRomInterpolate(ctrlPoints, subdivisions);
}

std::vector<Coordinate> ObstacleAvoidancePathfinder::collisionFreeNudge(
    const std::vector<Coordinate>& path,
    const std::vector<Obstacle*>& obstacles,
    double safetyMarginKm)
{
    if (path.size() < 2) return path;

    std::vector<Coordinate> nudged = path;
    for (int iter = 0; iter < 10; ++iter) {
        bool anyCollision = false;
        for (size_t i = 0; i < nudged.size(); ++i) {
            for (auto* obs : obstacles) {
                double dist = crossTrackDistance(obs->position,
                    nudged[i], nudged[std::min(i + 1, nudged.size() - 1)]);
                if (dist < obs->radius + safetyMarginKm && dist > 0.01) {
                    double bearing = obs->position.bearingTo(nudged[i]);
                    double pushDist = (obs->radius + safetyMarginKm - dist) * 0.3;
                    Coordinate pushDir = Coordinate(
                        nudged[i].latitude + pushDist * std::cos(bearing * M_PI / 180.0),
                        nudged[i].longitude + pushDist * std::sin(bearing * M_PI / 180.0),
                        nudged[i].altitude
                    );
                    nudged[i] = pushDir;
                    anyCollision = true;
                }
            }
        }
        if (!anyCollision) break;
    }
    return nudged;
}

std::vector<Coordinate> ObstacleAvoidancePathfinder::fallbackPathRRT(
    const Coordinate& start, const Coordinate& goal,
    const std::vector<Obstacle*>& obstacles)
{
    std::vector<Coordinate> nodes = {start, goal};
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> latDist(
        std::min(start.latitude, goal.latitude) - 5.0,
        std::max(start.latitude, goal.latitude) + 5.0);
    std::uniform_real_distribution<double> lonDist(
        std::min(start.longitude, goal.longitude) - 5.0,
        std::max(start.longitude, goal.longitude) + 5.0);

    for (int i = 0; i < 500; ++i) {
        double rLat = latDist(gen);
        double rLon = lonDist(gen);
        Coordinate sample(rLat, rLon, 0.0);

        int nearestIdx = 0;
        double nearestDist = std::numeric_limits<double>::max();
        for (size_t j = 0; j < nodes.size(); ++j) {
            double d = sample.distanceTo(nodes[j]);
            if (d < nearestDist) {
                nearestDist = d;
                nearestIdx = static_cast<int>(j);
            }
        }

        double step = 2.0;
        double bearing = nodes[nearestIdx].bearingTo(sample);
        double brngRad = bearing * M_PI / 180.0;
        Coordinate newPoint(
            nodes[nearestIdx].latitude + step * std::cos(brngRad),
            nodes[nearestIdx].longitude + step * std::sin(brngRad),
            0.0
        );

        bool collision = false;
        for (auto* obs : obstacles) {
            double dist = crossTrackDistance(obs->position, nodes[nearestIdx], newPoint);
            if (dist < obs->radius + config_.safetyMarginKm) {
                collision = true;
                break;
            }
        }
        if (!collision) {
            nodes.push_back(newPoint);
            double distToGoal = newPoint.distanceTo(goal);
            if (distToGoal < 3.0) {
                nodes.push_back(goal);
                break;
            }
        }
    }

    return nodes;
}

ObstacleAvoidancePathfinder::PathResult ObstacleAvoidancePathfinder::computePath(
    const Coordinate& start, const Coordinate& goal,
    const std::vector<Obstacle*>& obstacles,
    const std::vector<NoFlyZone*>& noFlyZones)
{
    PathResult result;

    auto blockers = findBlockingObstacles(start, goal, obstacles, noFlyZones, config_.safetyMarginKm);
    if (blockers.empty()) {
        result.waypoints = {start, goal};
        result.totalDistanceKm = start.distanceTo(goal);
        result.isValid = true;
        return result;
    }

    auto rawNodes = generateAnchorGraph(start, goal, blockers);
    if (rawNodes.empty()) {
        auto rrtPath = fallbackPathRRT(start, goal, obstacles);
        if (!rrtPath.empty()) {
            result.waypoints = catmullRomSmooth(rrtPath, config_.smoothSubdivisions);
            result.totalDistanceKm = 0.0;
            for (size_t i = 1; i < result.waypoints.size(); ++i) {
                result.totalDistanceKm += result.waypoints[i - 1].distanceTo(result.waypoints[i]);
            }
            result.isValid = true;
        }
        return result;
    }

    AnchorPointGraph graph;
    int startId = graph.addNode(start, "start", "");
    std::vector<int> prevIds = {startId};

    for (const auto& blocker : blockers) {
        Coordinate center = blocker.obs ? blocker.obs->position : blocker.zone->center;
        double radius = blocker.obs ? blocker.obs->radius : blocker.zone->radius;
        TangentBypass bypass = computeTangentBypass(center, radius + config_.safetyMarginKm, start, goal);

        std::vector<int> currIds;
        currIds.push_back(graph.addNode(bypass.tangentLeft, std::to_string(blocker.fractionAlong), "left"));
        currIds.push_back(graph.addNode(bypass.tangentRight, std::to_string(blocker.fractionAlong), "right"));

        for (int p : prevIds) {
            for (int c : currIds) {
                graph.addEdge(p, c, graph.nodes()[p].coord.distanceTo(graph.nodes()[c].coord));
            }
        }
        prevIds = currIds;
    }

    int goalId = graph.addNode(goal, "goal", "");
    for (int p : prevIds) {
        graph.addEdge(p, goalId, graph.nodes()[p].coord.distanceTo(graph.nodes()[goalId].coord));
    }

    auto pathCoords = graph.findPath(startId, goalId);
    if (pathCoords.empty()) {
        auto rrtPath = fallbackPathRRT(start, goal, obstacles);
        if (!rrtPath.empty()) {
            result.waypoints = catmullRomSmooth(rrtPath, config_.smoothSubdivisions);
            result.isValid = true;
        }
        return result;
    }

    result.rawNodes = graph.nodes();

    auto smoothed = catmullRomSmooth(pathCoords, config_.smoothSubdivisions);
    result.waypoints = smoothed;

    result.totalDistanceKm = 0.0;
    for (size_t i = 1; i < result.waypoints.size(); ++i) {
        result.totalDistanceKm += result.waypoints[i - 1].distanceTo(result.waypoints[i]);
    }
    result.isValid = true;

    return result;
}
