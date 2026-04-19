#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <random>
#include <map>
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
extern bool g_ui_paramsUpdated;
extern double g_ui_speed;
extern double g_ui_batteryDrain;
extern std::vector<Obstacle*> g_ui_newObstacles;

struct SpawnDroneEvent { double lat; double lon; };
extern std::vector<SpawnDroneEvent> g_ui_spawnDrones;

struct DroneContext {
    UAV uav;
    Coordinate startCoord;
    Coordinate targetCoord;
};

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
    
    // Phase 2: Obstacle Generation
    std::vector<Obstacle*> obstacles;
    obstacles.push_back(new StaticObstacle(Coordinate(48.0, 2.0, 0.0), 5.0));
    obstacles.push_back(new DynamicObstacle(Coordinate(47.5, 2.1, 0.0), 2.0, 1.5, 90.0));
    
    // Phase 3: Local Avoidance Initialization
    LocalAvoidance avoidance(10.0, 60.0); // 10km sensor range, 60 degree FOV

    // Phase 5: Swarm Architecture initialization
    std::map<std::string, DroneContext> swarm;
    // Give the Alpha drone a snappy turn rate of 90.0 deg/s
    swarm.insert({"alpha", { UAV(startCoord, 45.0, 0.25, 90.0), startCoord, targetCoord }});

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
                for (auto& pair : swarm) {
                    pair.second.targetCoord = targetCoord;
                }
                g_ui_targetUpdated = false;
                cout << ">>> NEW TARGET ASSIGNED: " << g_ui_targetLat << ", " << g_ui_targetLon << " <<<" << endl;
            }
            if (g_ui_paramsUpdated) {
                for (auto& pair : swarm) {
                    pair.second.uav.speed = g_ui_speed;
                    pair.second.uav.batteryDrainRate = g_ui_batteryDrain;
                }
                g_ui_paramsUpdated = false;
            }
            if (!g_ui_newObstacles.empty()) {
                for (auto* obs : g_ui_newObstacles) {
                    obstacles.push_back(obs);
                    server.broadcast(SimulationServer::formatObstacleState(
                        obs->isDynamic ? "dynamic" : "static",
                        "obs_" + to_string(obstacles.size()),
                        obs->position.latitude, obs->position.longitude, obs->radius, obs->isDynamic
                    ));
                    cout << ">>> UI ASSIGNED OBSTACLE SPAWNED <<<" << endl;
                }
                g_ui_newObstacles.clear();
            }
            if (!g_ui_spawnDrones.empty()) {
                for (auto& spawn : g_ui_spawnDrones) {
                    Coordinate spawnCoord(spawn.lat, spawn.lon, 0.0);
                    std::string id = "drone_" + to_string(swarm.size() + 1);
                    swarm.insert({id, { UAV(spawnCoord, 0.0, g_ui_speed, 90.0), spawnCoord, swarm.at("alpha").targetCoord }});
                    cout << ">>> NEW DRONE SPAWNED: " << id << " <<<" << endl;
                }
                g_ui_spawnDrones.clear();
            }
        }

        // 1. Process Swarm Updates
        for (auto& pair : swarm) {
            auto& ctx = pair.second;
            auto& uav = ctx.uav;

            double distToTarget = uav.position.distanceTo(ctx.targetCoord);
            if (distToTarget >= 0.02) { 
                double targetHeading = uav.position.bearingTo(ctx.targetCoord);

                for (auto* obs : obstacles) {
                    if (avoidance.isObstacleInPath(uav.position, targetHeading, obs)) {
                        targetHeading = avoidance.recalculateHeading(uav.position, targetHeading, obs);
                        break; 
                    }
                }

                uav.adjustHeading(targetHeading, deltaTime);
                
                double diff = std::abs(std::fmod(targetHeading - uav.heading + 360.0, 360.0));
                if (diff > 180.0) diff = 360.0 - diff;

                if (diff > 25.0) {
                    uav.speed = 0.0; // Halt to execute turn
                } else {
                    uav.speed = g_ui_speed; // Clear to fly
                }

                uav.update(deltaTime, 0.0);
            } else {
                uav.position = ctx.targetCoord;
                uav.speed = 0.0;
            }

            // Broadcast state via WebSocket
            string stateJson = SimulationServer::formatUAVState(
                pair.first,
                uav.position.latitude, 
                uav.position.longitude, 
                0.0, 
                uav.heading, 
                uav.batteryLevel,
                ctx.startCoord.latitude,
                ctx.startCoord.longitude,
                ctx.targetCoord.latitude,
                ctx.targetCoord.longitude
            );
            server.broadcast(stateJson);
        }

        // 2. Update dynamic obstacles
        for (auto* obs : obstacles) {
            if (obs->isDynamic) {
                obs->updatePosition(deltaTime);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Tick at 20Hz for fluid movement
    }

    for (auto* obs : obstacles) {
        delete obs;
    }

    server.stop();
    return 0;
}
