# C++ Pathfinder API Contract

## Class: `ObstacleAvoidancePathfinder`

### Header: `include/Pathfinder.h`

```cpp
class ObstacleAvoidancePathfinder {
public:
    struct AnchorNode {
        int id;
        Coordinate coord;
        std::string tag;    // "start", "goal", or obstacle ID
        std::string side;   // "left", "right", or ""
    };

    struct PathResult {
        std::vector<Coordinate> waypoints;
        std::vector<AnchorNode> rawNodes;
        double totalDistanceKm;
        bool isValid;
    };

    ObstacleAvoidancePathfinder(const ObstacleAvoidanceConfig& config);

    // Primary entry point — computes obstacle-free smoothed path
    PathResult computePath(
        const Coordinate& start,
        const Coordinate& goal,
        const std::vector<Obstacle*>& obstacles,
        const std::vector<NoFlyZone*>& noFlyZones = {}
    );

private:
    struct Blocker {
        const Obstacle* obs;
        const NoFlyZone* zone;    // one of obs or zone is non-null
        double crossTrackDist;
        double fractionAlong;     // 0.0=start, 1.0=goal
    };

    std::vector<Blocker> findBlockingObstacles(
        const Coordinate& start,
        const Coordinate& goal,
        const std::vector<Obstacle*>& obstacles,
        const std::vector<NoFlyZone*>& zones,
        double safetyMarginKm
    );

    std::vector<AnchorNode> generateAnchorGraph(
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
```

## Class: `AnchorPointGraph`

### Header: `include/AnchorPointGraph.h`

```cpp
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

    int addNode(const Coordinate& coord, const std::string& tag, const std::string& side);
    void addEdge(int from, int to, double cost);
    void connectLayer(int fromLayerStart, int fromLayerEnd, int toLayerStart, int toLayerEnd,
                      const std::vector<Obstacle*>& obstacles, double safetyMargin);

    std::vector<Coordinate> findPath(int startId, int goalId);

private:
    std::vector<AnchorNode> nodes_;
    std::vector<std::vector<Edge>> adjList_;
};
```

## ENU Utility Functions

### Header: `include/Geo.h` (additions)

```cpp
// Convert ECEF to local East-North-Up coordinates relative to origin
Vector3 ecefToEnu(const Vector3& pointEcef, const Vector3& originEcef);

// Convert local ENU back to geographic Coordinate
Coordinate enuToCoordinate(double east, double north, double up, const Coordinate& origin);

// Cross-track distance from point to great-circle segment
double crossTrackDistance(const Coordinate& point, const Coordinate& segStart, const Coordinate& segEnd);

// Fraction along great-circle segment where perpendicular projection falls
double fractionAlong(const Coordinate& segStart, const Coordinate& segEnd, const Coordinate& point);

// Tangent bypass points for a circular obstacle in ENU frame
struct TangentBypass {
    Coordinate approachLeft, tangentLeft, exitLeft;
    Coordinate approachRight, tangentRight, exitRight;
};
TangentBypass computeTangentBypass(const Coordinate& obsCenter, double radiusKm,
                                    const Coordinate& from, const Coordinate& to);
```
