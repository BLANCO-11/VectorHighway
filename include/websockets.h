#pragma once

#include <string>
#include <functional>
#include <vector>

/**
 * @brief Interface for the WebSocket Server to broadcast simulation state.
 * In a full implementation, this would wrap a library like websocketpp or boost::beast.
 */
class SimulationServer {
public:
    virtual ~SimulationServer() = default;

    // Starts the server on a specific port
    virtual bool start(int port) = 0;

    // Stops the server
    virtual void stop() = 0;

    // Broadcasts a JSON-formatted string to all connected clients
    virtual void broadcast(const std::string& message) = 0;

    // Handles incoming commands from the client (e.g., SET_TARGET)
    virtual void handleCommand(const std::string& commandJson) = 0;

    // Utility to create a simple JSON string for UAV state
    // Format: {"type": "uav_update", "lat": 0.0, "lon": 0.0, "alt": 0.0, "heading": 0.0, "battery": 100.0, "startLat": 0.0, "startLon": 0.0, "targetLat": 0.0, "targetLon": 0.0}
    static std::string formatUAVState(const std::string& id, const std::string& groupId, double lat, double lon, double alt, double heading, double battery, double startLat, double startLon, double targetLat, double targetLon);

    // Utility to create a JSON string for obstacle updates
    // Format: {"type": "obstacle_update", "id": "id", "lat": 0.0, "lon": 0.0, "radius": 0.0, "dynamic": true/false}
    static std::string formatObstacleState(const std::string& type, const std::string& id, double lat, double lon, double rad, bool dynamic, const std::string& groupId);
};

/**
 * @brief A Mock implementation of the SimulationServer for testing/development 
 * without external network dependencies.
 */
class MockSimulationServer : public SimulationServer {
public:
    bool start(int port) override;
    void stop() override;
    void broadcast(const std::string& message) override;
    void handleCommand(const std::string& commandJson) override;

    static std::string formatUAVState(const std::string& id, const std::string& groupId, double lat, double lon, double alt, double heading, double battery, double startLat, double startLon, double targetLat, double targetLon);
    static std::string formatObstacleState(const std::string& type, const std::string& id, double lat, double lon, double rad, bool dynamic, const std::string& groupId);
};
