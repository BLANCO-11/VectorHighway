#pragma once

#include "Geo.h"
#include "Environment.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <iostream>

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

        return diff <= (fieldOfView / 2.0);
    }

    // Returns an evasive heading away from the obstacle
    double recalculateHeading(const Coordinate& hostPos, double hostHeading, const Obstacle* obs) {
        double bearing = hostPos.bearingTo(obs->position);
        double diff = std::fmod(hostHeading - bearing + 360.0, 360.0);
        
        if (diff < 180.0) {
            // Obstacle is to the right, steer left
            std::cout << "[Local Avoidance] Threat detected at bearing " << bearing << ". Steering left 45 deg." << std::endl;
            return std::fmod(hostHeading - 45.0 + 360.0, 360.0); 
        } else {
            // Obstacle is to the left, steer right
            std::cout << "[Local Avoidance] Threat detected at bearing " << bearing << ". Steering right 45 deg." << std::endl;
            return std::fmod(hostHeading + 45.0, 360.0);
        }
    }
};