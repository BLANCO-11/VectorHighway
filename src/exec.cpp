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

struct CommandEvent {
    std::string type;
    std::string targetId;
    double lat, lon, radius, speed, drain;
    std::string groupId;
};

extern std::mutex g_ui_simMtx;
extern std::vector<CommandEvent> g_ui_commands;

struct ExternalTelemetry { std::string id; std::string groupId; double lat; double lon; double alt; double heading; double battery; };
extern std::vector<ExternalTelemetry> g_ui_externalTelemetry;

struct DroneContext {
    UAV uav;
    Coordinate startCoord;
    Coordinate targetCoord;
    double assignedSpeed = 0.25;
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
    swarm.insert({"alpha_1", { UAV(startCoord, 45.0, 0.25, 90.0, "alpha"), startCoord, targetCoord, 0.25 }});
    swarm.insert({"bravo_1", { UAV(Coordinate(26.88, 75.80, 0.0), 0.0, 0.25, 90.0, "bravo"), Coordinate(26.88, 75.80, 0.0), targetCoord, 0.25 }});

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
            for (const auto& cmd : g_ui_commands) {
                if (cmd.type == "target") {
                    Coordinate tgt(cmd.lat, cmd.lon, 0.0);
                    for (auto& pair : swarm) {
                        if (cmd.targetId == "all" || pair.second.uav.groupId == cmd.targetId || pair.first == cmd.targetId) {
                            pair.second.targetCoord = tgt;
                        }
                    }
                } else if (cmd.type == "control") {
                    for (auto& pair : swarm) {
                        if (cmd.targetId == "all" || pair.second.uav.groupId == cmd.targetId || pair.first == cmd.targetId) {
                            if (cmd.speed >= 0.0) pair.second.assignedSpeed = cmd.speed;
                            if (cmd.drain >= 0.0) pair.second.uav.batteryDrainRate = cmd.drain;
                        }
                    }
                } else if (cmd.type == "spawn") {
                    Coordinate spawnCoord(cmd.lat, cmd.lon, 0.0);
                    std::string id = "drone_" + to_string(swarm.size() + 1);
                    Coordinate initialTarget = spawnCoord;
                    for (const auto& pair : swarm) {
                        if (pair.second.uav.groupId == cmd.groupId) {
                            initialTarget = pair.second.targetCoord;
                            break;
                        }
                    }
                    swarm.insert({id, { UAV(spawnCoord, 0.0, 0.25, 90.0, cmd.groupId), spawnCoord, initialTarget, 0.25 }});
                    cout << ">>> NEW DRONE SPAWNED: " << id << " IN GROUP " << cmd.groupId << " <<<" << endl;
                } else if (cmd.type == "obstacle") {
                    Obstacle* obs = new StaticObstacle(Coordinate(cmd.lat, cmd.lon, 0.0), cmd.radius, cmd.groupId);
                    obstacles.push_back(obs);
                    server.broadcast(SimulationServer::formatObstacleState(
                        "static", "obs_" + to_string(obstacles.size()), cmd.lat, cmd.lon, cmd.radius, false, cmd.groupId
                    ));
                }
            }
            g_ui_commands.clear();
            
            // Phase 4: Forward external MAVLink/real-world drones telemetry
            for (const auto& ext : g_ui_externalTelemetry) {
                string stateJson = SimulationServer::formatUAVState(
                    ext.id, ext.groupId, ext.lat, ext.lon, ext.alt, ext.heading, ext.battery,
                    ext.lat, ext.lon, ext.lat, ext.lon // Real drones just report current pos as start/target
                );
                
                server.broadcast(stateJson);
            }
            g_ui_externalTelemetry.clear();
        }

        // 1. Process Swarm Updates
        for (auto& pair : swarm) {
            auto& ctx = pair.second;
            auto& uav = ctx.uav;

            double distToTarget = uav.position.distanceTo(ctx.targetCoord);
            double frameTravel = std::max(ctx.assignedSpeed, uav.currentSpeed) * deltaTime;
            
            // Corrected Velocity Tracking - Enforce perfect arrival without overshoot
            if (distToTarget <= frameTravel * 1.5) { 
                uav.position = ctx.targetCoord;
                uav.targetSpeed = 0.0;
                uav.currentSpeed = 0.0;
            } else {
                double targetHeading = uav.position.bearingTo(ctx.targetCoord);

                for (auto* obs : obstacles) {
                    if (obs->groupId.empty() || obs->groupId == uav.groupId) {
                        if (avoidance.isObstacleInPath(uav.position, targetHeading, obs)) {
                            targetHeading = avoidance.recalculateHeading(uav.position, targetHeading, obs);
                            break; 
                        }
                    }
                }

                uav.adjustHeading(targetHeading, deltaTime);
                
                double diff = std::abs(std::fmod(targetHeading - uav.heading + 360.0, 360.0));
                if (diff > 180.0) diff = 360.0 - diff;

                if (diff > 25.0) {
                    uav.targetSpeed = 0.0; // Halt to execute turn
                } else {
                    uav.targetSpeed = ctx.assignedSpeed; // Clear to fly
                }

                uav.update(deltaTime, 0.0);
            }

            // Broadcast state via WebSocket
            string stateJson = SimulationServer::formatUAVState(
                pair.first, uav.groupId,
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
