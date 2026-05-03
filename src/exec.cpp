#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <random>
#include <map>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include "../include/Geo.h"

using json = nlohmann::json;
#include "../include/Environment.h"
#include "../include/Pathfinder.h"
#include "../include/AnchorPointGraph.h"
#include "../include/UAV.h"
#include "../include/websockets.h"
#include "../include/SimulationContext.h"
#include "../include/Types.h"

using namespace std;

vector<Obstacle*> collectPtrs(const vector<unique_ptr<Obstacle>>& obstacles) {
    vector<Obstacle*> ptrs;
    for (const auto& obs : obstacles) ptrs.push_back(obs.get());
    return ptrs;
}

void broadcastPathUpdate(MockSimulationServer& server, const string& droneId, const vector<Coordinate>& path) {
    stringstream ss;
    ss << "{\"type\": \"path_update\", \"droneId\": \"" << droneId << "\", \"path\": [";
    for (size_t i = 0; i < path.size(); ++i) {
        if (i > 0) ss << ",";
        ss << "{\"lat\": " << fixed << setprecision(6) << path[i].latitude
           << ",\"lon\": " << path[i].longitude << "}";
    }
    ss << "]}";
    server.broadcast(ss.str());
}

struct DronePathState {
    ObstacleAvoidancePathfinder::PathResult path;
    double replanTimer = 0.0;
    size_t currentWaypointIndex = 0;
    Coordinate previousTarget;
};

class ObstacleGenerator {
public:
    ObstacleGenerator(double latMin, double latMax, double lonMin, double lonMax)
        : latMin(latMin), latMax(latMax), lonMin(lonMin), lonMax(lonMax), gen(rd()) {}

    unique_ptr<Obstacle> generateStatic(double radius) {
        uniform_real_distribution<> latDist(latMin, latMax);
        uniform_real_distribution<> lonDist(lonMin, lonMax);
        return make_unique<StaticObstacle>(Coordinate(latDist(gen), lonDist(gen), 0.0), radius);
    }

    unique_ptr<Obstacle> generateDynamic(double radius, double speed, double heading) {
        uniform_real_distribution<> latDist(latMin, latMax);
        uniform_real_distribution<> lonDist(lonMin, lonMax);
        return make_unique<DynamicObstacle>(Coordinate(latDist(gen), lonDist(gen), 0.0), radius, speed, heading);
    }

private:
    double latMin, latMax, lonMin, lonMax;
    random_device rd;
    mt19937 gen;
};

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = atoi(argv[i + 1]);
            ++i;
        }
    }

    const char* envPort = getenv("UAV_SIM_PORT");
    if (envPort) {
        port = atoi(envPort);
    }

    SimulationContext context;

    MockSimulationServer server;
    server.start(port, &context);

    Coordinate targetCoord(48.423366, 2.345332, 0.0);
    Coordinate startCoord(26.870601, 75.794110, 0.0);

    vector<unique_ptr<Obstacle>> obstacles;
    obstacles.push_back(make_unique<StaticObstacle>(Coordinate(48.0, 2.0, 0.0), 5.0));
    obstacles.push_back(make_unique<DynamicObstacle>(Coordinate(47.5, 2.1, 0.0), 2.0, 1.5, 90.0));

    ObstacleAvoidanceConfig pathConfig;
    pathConfig.safetyMarginKm = 0.5;
    pathConfig.replanInterval = 2.0;
    pathConfig.smoothSubdivisions = 5;
    ObstacleAvoidancePathfinder pathfinder(pathConfig);

    LocalAvoidance avoidance(10.0, 60.0);

    map<string, DroneContext> swarm;
    map<string, DronePathState> dronePaths;
    swarm.try_emplace("alpha_1", make_unique<UAV>(startCoord, 45.0, 0.25, 90.0, "alpha"), startCoord, targetCoord, 0.25);
    swarm.try_emplace("bravo_1", make_unique<UAV>(Coordinate(26.88, 75.80, 0.0), 0.0, 0.25, 90.0, "bravo"), Coordinate(26.88, 75.80, 0.0), targetCoord, 0.25);

    cout << "Starting Simulation Loop on port " << port << "..." << endl;

    auto lastTime = std::chrono::high_resolution_clock::now();
    while (true) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        double deltaTime = std::chrono::duration<double>(currentTime - lastTime).count();
        if (deltaTime <= 0) deltaTime = 0.1;
        lastTime = currentTime;

        {
            auto cmds = context.drainCommands();
            for (const auto& cmd : cmds) {
                if (cmd.type == "target") {
                    cout << "[cmd] TARGET: id=" << cmd.targetId << " lat=" << cmd.lat << " lon=" << cmd.lon << endl;
                    Coordinate tgt(cmd.lat, cmd.lon, 0.0);
                    for (auto& pair : swarm) {
                        if (cmd.targetId == "all" || pair.second.uav->groupId == cmd.targetId || pair.first == cmd.targetId) {
                            pair.second.targetCoord = tgt;
                            auto result = pathfinder.computePath(
                                pair.second.uav->position, tgt, collectPtrs(obstacles), {}
                            );
                            auto& dps = dronePaths[pair.first];
                            dps.path = result;
                            dps.currentWaypointIndex = 0;
                            dps.replanTimer = 0.0;
                            if (result.isValid && !result.waypoints.empty()) {
                                broadcastPathUpdate(server, pair.first, result.waypoints);
                            }
                        }
                    }
                } else if (cmd.type == "control") {
                    for (auto& pair : swarm) {
                        if (cmd.targetId == "all" || pair.second.uav->groupId == cmd.targetId || pair.first == cmd.targetId) {
                            if (cmd.speed >= 0.0) pair.second.assignedSpeed = cmd.speed;
                            if (cmd.drain >= 0.0) pair.second.uav->batteryDrainRate = cmd.drain;
                        }
                    }
                } else if (cmd.type == "spawn") {
                    Coordinate spawnCoord(cmd.lat, cmd.lon, 0.0);
                    string id = "drone_" + to_string(swarm.size() + 1);
                    Coordinate initialTarget = spawnCoord;
                    for (const auto& pair : swarm) {
                        if (pair.second.uav->groupId == cmd.groupId) {
                            initialTarget = pair.second.targetCoord;
                            break;
                        }
                    }
                    swarm.try_emplace(id, make_unique<UAV>(spawnCoord, 0.0, 0.25, 90.0, cmd.groupId), spawnCoord, initialTarget, 0.25);
                    cout << ">>> NEW DRONE SPAWNED: " << id << " IN GROUP " << cmd.groupId << " <<<" << endl;
                } else if (cmd.type == "obstacle") {
                    auto obs = make_unique<StaticObstacle>(Coordinate(cmd.lat, cmd.lon, 0.0), cmd.radius, cmd.groupId);
                    server.broadcast(SimulationServer::formatObstacleState(
                        "static", "obs_" + to_string(obstacles.size() + 1), cmd.lat, cmd.lon, cmd.radius, false, cmd.groupId
                    ));
                    obstacles.push_back(std::move(obs));
                } else if (cmd.type == "remove") {
                    string id = cmd.targetId;
                    int idx = -1;
                    for (size_t i = 0; i < obstacles.size(); ++i) {
                        if (id == "obs_" + to_string(i + 1)) { idx = static_cast<int>(i); break; }
                    }
                    if (idx >= 0) {
                        obstacles.erase(obstacles.begin() + idx);
                        json rsp;
                        rsp["type"] = "obstacle_removed";
                        rsp["id"] = id;
                        server.broadcast(rsp.dump());
                    }
                } else if (cmd.type == "clear_group") {
                    string gid = cmd.groupId;
                    obstacles.erase(remove_if(obstacles.begin(), obstacles.end(),
                        [&gid](const unique_ptr<Obstacle>& o) { return o->groupId == gid; }),
                        obstacles.end());
                    json rsp;
                    rsp["type"] = "environment_cleared";
                    rsp["groupId"] = gid;
                    server.broadcast(rsp.dump());
                } else if (cmd.type == "clear_all") {
                    obstacles.clear();
                    json rsp;
                    rsp["type"] = "environment_cleared";
                    server.broadcast(rsp.dump());
                }
            }

            auto telemetryPackets = context.drainExternalTelemetry();
            for (const auto& ext : telemetryPackets) {
                context.updateExternalDrone(ext);
            }
        }

        {
            auto stale = context.getStaleDrones();
            for (const auto& id : stale) {
                cout << ">>> STALE DRONE REMOVED: " << id << " (30s timeout) <<<" << endl;
            }
        }

        {
            auto activeExt = context.getActiveExternalDrones();
            for (const auto& drone : activeExt) {
                const auto& ext = drone.telemetry;
                string stateJson = SimulationServer::formatUAVState(
                    ext.id, ext.groupId, ext.lat, ext.lon, ext.alt, ext.heading, ext.battery,
                    ext.lat, ext.lon, ext.lat, ext.lon
                );
                // Replace closing brace with isExternal flag and speed/status
                stateJson.pop_back();
                stateJson += ",\"speed\": " + std::to_string(ext.speed) +
                    ",\"isExternal\": true,\"status\": \"" + ext.status + "\"}";
                server.broadcast(stateJson);
            }
        }

        for (auto& pair : swarm) {
            auto& ctx = pair.second;
            auto& uav = *ctx.uav;
            auto& dps = dronePaths[pair.first];

            double distToTarget = uav.position.distanceTo(ctx.targetCoord);
            double frameTravel = max(ctx.assignedSpeed, uav.currentSpeed) * deltaTime;

            bool pathChanged = false;
            if (dps.previousTarget.latitude != ctx.targetCoord.latitude ||
                dps.previousTarget.longitude != ctx.targetCoord.longitude) {
                pathChanged = true;
                dps.previousTarget = ctx.targetCoord;
            }

            if (dps.path.isValid && !dps.path.waypoints.empty()) {
                dps.replanTimer += deltaTime;
                if (dps.replanTimer >= pathConfig.replanInterval) {
                    dps.replanTimer = 0.0;
                    auto result = pathfinder.computePath(
                        uav.position, ctx.targetCoord, collectPtrs(obstacles), {}
                    );
                    if (result.isValid) {
                        dps.path = result;
                        broadcastPathUpdate(server, pair.first, result.waypoints);
                    }
                }

                if (dps.currentWaypointIndex < dps.path.waypoints.size()) {
                    Coordinate& nextWp = dps.path.waypoints[dps.currentWaypointIndex];
                    double distToWp = uav.position.distanceTo(nextWp);

                    if (distToWp <= frameTravel * 1.5) {
                        dps.currentWaypointIndex++;
                        if (dps.currentWaypointIndex >= dps.path.waypoints.size()) {
                            uav.position = ctx.targetCoord;
                            uav.targetSpeed = 0.0;
                            uav.currentSpeed = 0.0;
                            goto broadcast_state;
                        }
                        nextWp = dps.path.waypoints[dps.currentWaypointIndex];
                    }

                    double targetHeading = uav.position.bearingTo(nextWp);
                    uav.adjustHeading(targetHeading, deltaTime);

                    double diff = abs(fmod(targetHeading - uav.heading + 360.0, 360.0));
                    if (diff > 180.0) diff = 360.0 - diff;

                    if (diff > 25.0) {
                        uav.targetSpeed = 0.0;
                    } else {
                        uav.targetSpeed = ctx.assignedSpeed;
                    }

                    uav.update(deltaTime, 0.0);
                } else {
                    uav.position = ctx.targetCoord;
                    uav.targetSpeed = 0.0;
                    uav.currentSpeed = 0.0;
                }
            } else {
                if (distToTarget <= frameTravel * 1.5) {
                    uav.position = ctx.targetCoord;
                    uav.targetSpeed = 0.0;
                    uav.currentSpeed = 0.0;
                } else {
                    double targetHeading = uav.position.bearingTo(ctx.targetCoord);

                    for (const auto& obs : obstacles) {
                        if (obs->groupId.empty() || obs->groupId == uav.groupId) {
                            if (avoidance.isObstacleInPath(uav.position, targetHeading, obs.get())) {
                                targetHeading = avoidance.recalculateHeading(uav.position, targetHeading, obs.get());
                                break;
                            }
                        }
                    }

                    uav.adjustHeading(targetHeading, deltaTime);

                    double diff = abs(fmod(targetHeading - uav.heading + 360.0, 360.0));
                    if (diff > 180.0) diff = 360.0 - diff;

                    if (diff > 25.0) {
                        uav.targetSpeed = 0.0;
                    } else {
                        uav.targetSpeed = ctx.assignedSpeed;
                    }

                    uav.update(deltaTime, 0.0);
                }
            }

            broadcast_state:
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

        for (auto& obs : obstacles) {
            if (obs->isDynamic) {
                obs->updatePosition(deltaTime);
            }
        }

        this_thread::sleep_for(chrono::milliseconds(static_cast<int>(1000.0 / SIMULATION_TICK_HZ)));
    }

    server.stop();
    return 0;
}
