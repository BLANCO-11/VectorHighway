#pragma once

#include "Geo.h"
#include "Environment.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include <cmath>

class AnchorPointGraph {
public:
    struct AnchorNode {
        int id;
        Coordinate coord;
        std::string tag;
        std::string side;
    };

    struct Edge {
        int to;
        double cost;
    };

    int addNode(const Coordinate& coord, const std::string& tag, const std::string& side = "") {
        int id = static_cast<int>(nodes_.size());
        nodes_.push_back({id, coord, tag, side});
        adjList_.push_back({});
        return id;
    }

    void addEdge(int from, int to, double cost) {
        adjList_[from].push_back({to, cost});
        adjList_[to].push_back({from, cost});
    }

    void connectLayer(int fromLayerStart, int fromLayerEnd, int toLayerStart, int toLayerEnd,
                      const std::vector<Obstacle*>& obstacles, double safetyMargin) {
        for (int i = fromLayerStart; i <= fromLayerEnd && i < static_cast<int>(nodes_.size()); ++i) {
            for (int j = toLayerStart; j <= toLayerEnd && j < static_cast<int>(nodes_.size()); ++j) {
                if (i == j) continue;
                if (!segmentIntersectsAnyObstacle(nodes_[i].coord, nodes_[j].coord, obstacles, safetyMargin)) {
                    double cost = nodes_[i].coord.distanceTo(nodes_[j].coord);
                    addEdge(i, j, cost);
                }
            }
        }
    }

    std::vector<Coordinate> findPath(int startId, int goalId) {
        std::vector<double> gScore(nodes_.size(), std::numeric_limits<double>::max());
        std::vector<int> cameFrom(nodes_.size(), -1);
        using Pair = std::pair<double, int>;
        std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> pq;

        auto heuristic = [&](int a, int b) {
            return nodes_[a].coord.distanceTo(nodes_[b].coord);
        };

        gScore[startId] = 0.0;
        pq.push({heuristic(startId, goalId), startId});

        while (!pq.empty()) {
            int current = pq.top().second;
            pq.pop();

            if (current == goalId) {
                std::vector<Coordinate> path;
                int curr = current;
                while (curr != -1) {
                    path.push_back(nodes_[curr].coord);
                    curr = cameFrom[curr];
                }
                std::reverse(path.begin(), path.end());
                return path;
            }

            for (const auto& edge : adjList_[current]) {
                double tentative = gScore[current] + edge.cost;
                if (tentative < gScore[edge.to]) {
                    gScore[edge.to] = tentative;
                    cameFrom[edge.to] = current;
                    pq.push({tentative + heuristic(edge.to, goalId), edge.to});
                }
            }
        }

        return {};
    }

    const std::vector<AnchorNode>& nodes() const { return nodes_; }
    const std::vector<std::vector<Edge>>& adjList() const { return adjList_; }

private:
    bool segmentIntersectsAnyObstacle(const Coordinate& a, const Coordinate& b,
                                      const std::vector<Obstacle*>& obstacles, double safetyMargin) {
        for (const auto* obs : obstacles) {
            double dist = crossTrackDistanceToObstacle(a, b, obs->position);
            if (dist < obs->radius + safetyMargin) {
                double fracA = a.distanceTo(obs->position);
                double fracB = b.distanceTo(obs->position);
                double segLen = a.distanceTo(b);
                if (fracA <= segLen && fracB <= segLen) return true;
            }
        }
        return false;
    }

    double crossTrackDistanceToObstacle(const Coordinate& a, const Coordinate& b, const Coordinate& obsPos) {
        return crossTrackDistance(obsPos, a, b);
    }

    std::vector<AnchorNode> nodes_;
    std::vector<std::vector<Edge>> adjList_;
};
