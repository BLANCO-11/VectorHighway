#pragma once

#include "Geo.h"
#include "Environment.h"
#include "AnchorPointGraph.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <iostream>

// ObstacleAvoidanceConfig
struct ObstacleAvoidanceConfig {
    double safetyMarginKm = 0.5;
    double replanInterval = 2.0;
    int smoothSubdivisions = 5;
    int maxAnchorPathSearch = 5000;
    double turnRateDegPerSec = 90.0;
};

// Phase 3: Global Routing using A* algorithm
class GlobalPathfinder {
public:
    struct Node {
        Coordinate coord;
        std::string name;
        int id;
    };

    struct Edge {
        int to;
        double cost;
    };

    std::vector<Node> nodes;
    std::vector<std::vector<Edge>> adjList;
    double maxFlightRange; // maximum distance the UAV can travel without charging

    GlobalPathfinder(double range) : maxFlightRange(range) {}

    int addNode(const Coordinate& coord, const std::string& name) {
        int id = static_cast<int>(nodes.size());
        nodes.push_back({coord, name, id});
        adjList.push_back({});
        return id;
    }

    void buildGraph() {
        for (size_t i = 0; i < nodes.size(); ++i) {
            adjList[i].clear();
        }
        // Build edges if the distance is within the max flight range
        for (size_t i = 0; i < nodes.size(); ++i) {
            for (size_t j = i + 1; j < nodes.size(); ++j) {
                double dist = nodes[i].coord.distanceTo(nodes[j].coord);
                if (dist <= maxFlightRange) {
                    adjList[i].push_back({static_cast<int>(j), dist});
                    adjList[j].push_back({static_cast<int>(i), dist});
                }
            }
        }
    }

    std::vector<Waypoint> findPathAStar(int startId, int goalId) {
        auto heuristic = [&](int a, int b) {
            return nodes[a].coord.distanceTo(nodes[b].coord);
        };

        std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<>> pq;
        std::unordered_map<int, double> gScore;
        std::unordered_map<int, int> cameFrom;

        for (size_t i = 0; i < nodes.size(); ++i) gScore[i] = 1e9;
        gScore[startId] = 0;

        pq.push({heuristic(startId, goalId), startId});

        while (!pq.empty()) {
            int current = pq.top().second;
            pq.pop();

            if (current == goalId) {
                std::vector<Waypoint> path;
                int curr = current;
                while (cameFrom.find(curr) != cameFrom.end()) {
                    path.push_back(Waypoint(nodes[curr].coord, nodes[curr].name));
                    curr = cameFrom[curr];
                }
                path.push_back(Waypoint(nodes[startId].coord, nodes[startId].name));
                std::reverse(path.begin(), path.end());
                return path;
            }

            for (const auto& edge : adjList[current]) {
                double tentative_gScore = gScore[current] + edge.cost;
                if (tentative_gScore < gScore[edge.to]) {
                    cameFrom[edge.to] = current;
                    gScore[edge.to] = tentative_gScore;
                    pq.push({gScore[edge.to] + heuristic(edge.to, goalId), edge.to});
                }
            }
        }

        return {}; // Return empty if no path is found
    }
};

// Phase 3: ObstacleAvoidancePathfinder — anchor-point predictive pathfinding
class ObstacleAvoidancePathfinder {
public:
    struct PathResult {
        std::vector<Coordinate> waypoints;
        std::vector<AnchorPointGraph::AnchorNode> rawNodes;
        double totalDistanceKm = 0.0;
        bool isValid = false;
    };

    ObstacleAvoidancePathfinder(const ObstacleAvoidanceConfig& config)
        : config_(config) {}

    PathResult computePath(
        const Coordinate& start,
        const Coordinate& goal,
        const std::vector<Obstacle*>& obstacles,
        const std::vector<NoFlyZone*>& noFlyZones = {}
    );

private:
    struct Blocker {
        const Obstacle* obs = nullptr;
        const NoFlyZone* zone = nullptr;
        double crossTrackDist = 0.0;
        double fractionAlong = 0.0;
    };

    std::vector<Blocker> findBlockingObstacles(
        const Coordinate& start,
        const Coordinate& goal,
        const std::vector<Obstacle*>& obstacles,
        const std::vector<NoFlyZone*>& zones,
        double safetyMarginKm
    );

    std::vector<AnchorPointGraph::AnchorNode> generateAnchorGraph(
        const Coordinate& start,
        const Coordinate& goal,
        const std::vector<Blocker>& blockers
    );

    std::vector<Coordinate> catmullRomSmooth(
        const std::vector<Coordinate>& ctrlPoints,
        int subdivisions
    );

    std::vector<Coordinate> collisionFreeNudge(
        const std::vector<Coordinate>& path,
        const std::vector<Obstacle*>& obstacles,
        double safetyMarginKm
    );

    std::vector<Coordinate> fallbackPathRRT(
        const Coordinate& start,
        const Coordinate& goal,
        const std::vector<Obstacle*>& obstacles
    );

    ObstacleAvoidanceConfig config_;
};

// Phase 3: Local Avoidance (Simulating D* Lite / Field of View Recalculation behavior)
class LocalAvoidance {
public:
    double sensorRange;
    double fieldOfView;

    LocalAvoidance(double range = 10.0, double fov = 60.0) 
        : sensorRange(range), fieldOfView(fov) {}

    bool isObstacleInPath(const Coordinate& hostPos, double hostHeading, const Obstacle* obs) {
        double distance = hostPos.distanceTo(obs->position);
        
        if (distance > sensorRange + obs->radius) return false;

        double bearing = hostPos.bearingTo(obs->position);
        double diff = std::abs(std::fmod(hostHeading - bearing + 360.0, 360.0));
        if (diff > 180.0) {
            diff = 360.0 - diff;
        }

        if (distance < obs->radius + 0.5) return true; // Emergency evasion if too close

        return diff <= (fieldOfView / 2.0);
    }

    // Returns an evasive heading away from the obstacle
    double recalculateHeading(const Coordinate& hostPos, double currentHeading, const Obstacle* obs) {
        double bearing = hostPos.bearingTo(obs->position);
        double diff = std::fmod(currentHeading - bearing + 360.0, 360.0);
        
        if (diff < 180.0) {
            // Obstacle is to the right, steer 90 degrees left of bearing
            return std::fmod(bearing - 90.0 + 360.0, 360.0); 
        } else {
            // Obstacle is to the left, steer 90 degrees right of bearing
            return std::fmod(bearing + 90.0, 360.0);
        }
    }
};