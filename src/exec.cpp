#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <random>
#include "../include/Geo.h"
#include "../include/Environment.h"
#include "../include/Pathfinder.h"
#include "../include/UAV.h"
#include "../include/websockets.h"

using namespace std;

// Externs to link with the globals declared in websockets.cpp
extern std::mutex g_ui_simMtx;
extern bool g_ui_targetUpdated;
extern double g_ui_targetLat;
extern double g_ui_targetLon;

// Class to manage random obstacle generation
class ObstacleGenerator {
public:
    ObstacleGenerator(double latMin, double latMax, double lonMin, double lonMax)
        : latMin(latMin), latMax(latMax), lonMin(lonMin), lonMax(lonMax), gen(rd()) {}

    Obstacle* generateStatic(double radius) {
        uniform_real_distribution<> latDist(latMin, latMax);
        uniform_real_distribution<> lonDist(lonMin, lonMax);
        return new StaticObstacle(Coordinate(latDist(gen), lonDist(gen), 0.0), radius);
    }

    Obstacle* generateDynamic(double radius, double speed, double heading) {
        uniform_real_distribution<> latDist(latMin, latMax);
        uniform_real_distribution<> lonDist(lonMin, lonMax);
        return new DynamicObstacle(Coordinate(latDist(gen), lonDist(gen), 0.0), radius, speed, heading);
    }

private:
    double latMin, latMax, lonMin, lonMax;
    random_device rd;
    mt19937 gen;
};

int main() {
    // Initialize Mock Server for Phase 5
    MockSimulationServer server;
    server.start(8080);

    // Define target coordinates
    Coordinate targetCoord(48.423366, 2.345332, 0.0);
    Coordinate startCoord(26.870601, 75.794110, 0.0);
    
    // Phase 1: Using Waypoint data structures
    std::vector<Waypoint> targets;
    targets.emplace_back(Waypoint(targetCoord, "Target 1"));

    // Phase 2: Obstacle Generation
    std::vector<Obstacle*> obstacles;
    obstacles.push_back(new StaticObstacle(Coordinate(48.0, 2.0, 0.0), 5.0));
    obstacles.push_back(new DynamicObstacle(Coordinate(47.5, 2.1, 0.0), 2.0, 1.5, 90.0));
    
    // Phase 4: UAV Agent Initialization
    // Speed: 0.25 km/s (900 km/h), Turn Rate: 15 deg/s
    UAV uav(startCoord, 45.0, 0.25, 15.0);

    // Phase 3: Local Avoidance Initialization
    LocalAvoidance avoidance(10.0, 60.0); // 10km sensor range, 60 degree FOV

    // Obstacle Generator for random spawning
    ObstacleGenerator obsGen(45.0, 50.0, 1.0, 4.0);
    auto lastObstacleSpawn = std::chrono::high_resolution_clock::now();

    // Set up a random number generator for obstacle types
    std::random_device rd_main;
    std::mt19937 gen_main(rd_main());
    std::uniform_int_distribution<> obs_type_dist(0, 1);

    // Initial broadcast
    server.broadcast(SimulationServer::formatUAVState(
        uav.position.latitude, uav.position.longitude, 0.0, uav.heading, uav.batteryLevel,
        startCoord.latitude, startCoord.longitude, targetCoord.latitude, targetCoord.longitude
    ));

    cout << "Starting Simulation Loop..." << endl;

    // Phase 4: Deterministic Tick-based Update Loop
    auto lastTime = std::chrono::high_resolution_clock::now();
    while (true) { // Infinite continuous loop for live UI interaction
        auto currentTime = std::chrono::high_resolution_clock::now();
        double deltaTime = std::chrono::duration<double>(currentTime - lastTime).count();
        if (deltaTime <= 0) deltaTime = 0.1; // Avoid zero/negative delta
        lastTime = currentTime;

        // 0. Check for UI Target Updates from WebSocket
        {
            std::lock_guard<std::mutex> lock(g_ui_simMtx);
            if (g_ui_targetUpdated) {
                targetCoord = Coordinate(g_ui_targetLat, g_ui_targetLon, 0.0);
                targets[0] = Waypoint(targetCoord, "UI Assigned Target");
                g_ui_targetUpdated = false;
                cout << ">>> NEW TARGET ASSIGNED: " << g_ui_targetLat << ", " << g_ui_targetLon << " <<<" << endl;
            }
        }

        // 1. Randomly spawn obstacles every ~30 seconds
        if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastObstacleSpawn).count() > 30) {
            int type = obs_type_dist(gen_main);
            Obstacle* newObs = (type == 0) ? obsGen.generateStatic(3.0) : obsGen.generateDynamic(2.0, 1.0, 45.0);
            obstacles.push_back(newObs);
            
            // Broadcast the new obstacle
            server.broadcast(SimulationServer::formatObstacleState(
                newObs->isDynamic ? "dynamic" : "static",
                "obs_" + to_string(obstacles.size()),
                newObs->position.latitude, newObs->position.longitude, newObs->radius, newObs->isDynamic
            ));
            
            lastObstacleSpawn = currentTime;
            cout << ">>> RANDOM OBSTACLE SPAWNED <<<" << endl;
        }

        // 2. Determine Target Heading (fly towards the first target)
        double targetHeading = uav.position.bearingTo(targets[0].coordinate);

        // 3. Check Local Environment for Obstacles
        for (auto* obs : obstacles) {
            if (avoidance.isObstacleInPath(uav.position, uav.heading, obs)) {
                targetHeading = avoidance.recalculateHeading(uav.position, uav.heading, obs);
                break; // Only react to the most immediate threat in this step
            }
        }

        // 4. Update UAV state
        uav.adjustHeading(targetHeading, deltaTime);
        uav.update(deltaTime, 0.0);

        // 5. Update dynamic obstacles in the environment
        for (auto* obs : obstacles) {
            if (obs->isDynamic) {
                obs->updatePosition(deltaTime);
            }
        }

        // 6. Phase 5: Broadcast state via WebSocket
        string stateJson = SimulationServer::formatUAVState(
            uav.position.latitude, 
            uav.position.longitude, 
            0.0, 
            uav.heading, 
            uav.batteryLevel,
            startCoord.latitude,
            startCoord.longitude,
            targetCoord.latitude,
            targetCoord.longitude
        );
        server.broadcast(stateJson);

        // 7. Print Status
        cout << "Pos: (" << uav.position.latitude << ", " << uav.position.longitude << "), "
             << "Hdg: " << (int)uav.heading << ", Bat: " << (int)uav.batteryLevel << "%" << endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Slow down loop for readability
    }

    for (auto* obs : obstacles) {
        delete obs;
    }

    server.stop();
    return 0;
}
