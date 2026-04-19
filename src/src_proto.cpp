#include <iostream>
#include <vector>
#include "Geo.h"
#include "Environment.h"
#include "Pathfinder.h"

using namespace std;

int main() {
    // Phase 1: Removed global variables MyLat, Mylon. Using local Coordinate.
    Coordinate myPosition(26.4829, 74.0000, 0.0);

    // Phase 1: Create proper classes for Waypoint
    std::vector<Waypoint> targets;
    targets.emplace_back(Coordinate(50.5957, 3.0776, 0.0), "Target 1");
    targets.emplace_back(Coordinate(49.3995, -2.4985, 0.0), "Target 2");
    targets.emplace_back(Coordinate(48.3679, -4.4290, 0.0), "Target 3");

    // Phase 2: Coordinate System Conversion
    Vector3 cartesianPos = CoordinateConversion::sphericalToCartesian(myPosition);
    cout << "My Position (Cartesian): X=" << cartesianPos.x << ", Y=" << cartesianPos.y << ", Z=" << cartesianPos.z << "\n" << endl;

    // Phase 2: Charging Stations
    std::vector<ChargingStation> stations;
    stations.emplace_back(Coordinate(25.2048, 55.2708, 0.0), "Dubai Charging Node", 20.0);
    stations.emplace_back(Coordinate(41.0082, 28.9784, 0.0), "Istanbul Charging Node", 15.0);
    stations.emplace_back(Coordinate(48.8566, 2.3522, 0.0), "Paris Charging Node", 10.0);
    stations.emplace_back(Coordinate(51.5074, -0.1278, 0.0), "London Charging Node", 15.0);

    // Phase 3: Global Routing using A*
    GlobalPathfinder router(5000.0); // 5000 km max range between stops to force routing logic
    
    int startNode = router.addNode(myPosition, "Start Position");
    int destNode = router.addNode(targets[1].coordinate, targets[1].name); // Route to Target 2
    
    // Add charging stations as graph nodes
    for (const auto& station : stations) {
        router.addNode(station.position, station.name);
    }
    
    router.buildGraph();
    
    std::vector<Waypoint> path = router.findPathAStar(startNode, destNode);
    
    cout << "--- Global Route Plan ---" << endl;
    if (path.empty()) {
        cout << "No valid route found within drone range constraints." << endl;
    } else {
        for (size_t i = 0; i < path.size(); ++i) {
            cout << "Step " << i + 1 << ": " << path[i].name 
                 << " (Lat: " << path[i].coordinate.latitude 
                 << ", Lon: " << path[i].coordinate.longitude << ")" << endl;
        }
    }
    cout << "-------------------------\n" << endl;

    return 0;
}
