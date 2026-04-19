#include "../include/websockets.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <set>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include "../include/Environment.h" // For Obstacle

using json = nlohmann::json;

typedef websocketpp::server<websocketpp::config::asio> server;

// Global thread-safe state to pass data to the main simulation thread
struct CommandEvent {
    std::string type;
    std::string targetId;
    double lat, lon, radius, speed, drain;
    std::string groupId;
};

std::mutex g_ui_simMtx;
std::vector<CommandEvent> g_ui_commands;
struct ExternalTelemetry { std::string id; std::string groupId; double lat; double lon; double alt; double heading; double battery; };
std::vector<ExternalTelemetry> g_ui_externalTelemetry;

class WSServerImpl {
public:
    server endpoint;
    std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> connections;
    std::mutex mtx;
    std::thread server_thread;

    WSServerImpl() {
        endpoint.clear_access_channels(websocketpp::log::alevel::all);
        endpoint.init_asio();

        endpoint.set_open_handler([this](websocketpp::connection_hdl hdl) {
            std::lock_guard<std::mutex> lock(mtx);
            connections.insert(hdl);
            std::cout << "[WebSocket] Client connected." << std::endl;
        });

        endpoint.set_close_handler([this](websocketpp::connection_hdl hdl) {
            std::lock_guard<std::mutex> lock(mtx);
            connections.erase(hdl);
            std::cout << "[WebSocket] Client disconnected." << std::endl;
        });

        endpoint.set_message_handler([this](websocketpp::connection_hdl hdl, server::message_ptr msg) {
            std::string payload = msg->get_payload();
            std::cout << "[WebSocket] Received command: " << payload << std::endl;
            try {
                auto j = json::parse(payload);
                
                // Phase 4: Pub/Sub routing emulation
                if (j.contains("topic")) {
                    std::string topic = j["topic"];
                    auto msgPayload = j["payload"];
                    
                    if (topic.find("cmd/fleet/") == 0 && topic.find("/target") != std::string::npos) {
                        std::lock_guard<std::mutex> lock(g_ui_simMtx);
                        size_t firstSlash = topic.find('/', 10);
                        std::string id = topic.substr(10, firstSlash - 10);
                        g_ui_commands.push_back({"target", id, msgPayload["lat"], msgPayload["lon"], 0.0, 0.0, 0.0, ""});
                    }
                    else if (topic.find("cmd/fleet/") == 0 && topic.find("/control") != std::string::npos) {
                        std::lock_guard<std::mutex> lock(g_ui_simMtx);
                        size_t firstSlash = topic.find('/', 10);
                        std::string id = topic.substr(10, firstSlash - 10);
                        double speed = msgPayload.contains("speed") ? msgPayload["speed"].get<double>() : -1;
                        double drain = msgPayload.contains("batteryDrain") ? msgPayload["batteryDrain"].get<double>() : -1;
                        g_ui_commands.push_back({"control", id, 0, 0, 0, speed, drain, ""});
                    }
                    else if (topic == "cmd/environment/obstacle") {
                        std::lock_guard<std::mutex> lock(g_ui_simMtx);
                        double radius = msgPayload.contains("radius") ? msgPayload["radius"].get<double>() : 2.0;
                        std::string groupId = msgPayload.contains("groupId") ? msgPayload["groupId"].get<std::string>() : "";
                        g_ui_commands.push_back({"obstacle", "", msgPayload["lat"], msgPayload["lon"], radius, 0, 0, groupId});
                    }
                    else if (topic == "cmd/fleet/spawn") {
                        std::lock_guard<std::mutex> lock(g_ui_simMtx);
                        std::string groupId = msgPayload.contains("groupId") ? msgPayload["groupId"].get<std::string>() : "alpha";
                        g_ui_commands.push_back({"spawn", "", msgPayload["lat"], msgPayload["lon"], 0, 0, 0, groupId});
                    }
                    else if (topic.find("telemetry/external/") == 0) {
                        std::string id = topic.substr(19);
                        std::lock_guard<std::mutex> lock(g_ui_simMtx);
                        g_ui_externalTelemetry.push_back({
                            id, msgPayload.contains("groupId") ? msgPayload["groupId"].get<std::string>() : "alpha",
                            msgPayload["lat"].get<double>(),
                            msgPayload["lon"].get<double>(),
                            msgPayload.contains("alt") ? msgPayload["alt"].get<double>() : 0.0,
                            msgPayload.contains("heading") ? msgPayload["heading"].get<double>() : 0.0,
                            msgPayload.contains("battery") ? msgPayload["battery"].get<double>() : 100.0
                        });
                    }
                }
            } catch (const std::exception& e) {
                std::cout << "[WebSocket] JSON Parse Error: " << e.what() << std::endl;
            }
        });
    }
};

static WSServerImpl* ws_instance = nullptr;

bool MockSimulationServer::start(int port) {
    std::cout << "[WebSocket Server] Starting on port " << port << "..." << std::endl;
    ws_instance = new WSServerImpl();
    
    ws_instance->endpoint.listen(port);
    ws_instance->endpoint.start_accept();

    ws_instance->server_thread = std::thread([]() {
        try {
            ws_instance->endpoint.run();
        } catch (const std::exception &e) {
            std::cout << "[WebSocket Error] " << e.what() << std::endl;
        }
    });

    return true;
}

void MockSimulationServer::stop() {
    if (ws_instance) {
        ws_instance->endpoint.stop_listening();
        ws_instance->endpoint.stop();
        if (ws_instance->server_thread.joinable()) {
            ws_instance->server_thread.join();
        }
        delete ws_instance;
        ws_instance = nullptr;
        std::cout << "[WebSocket Server] Stopped." << std::endl;
    }
}

void MockSimulationServer::broadcast(const std::string& message) {
    if (!ws_instance) return;
    
    std::lock_guard<std::mutex> lock(ws_instance->mtx);
    for (auto it : ws_instance->connections) {
        websocketpp::lib::error_code ec;
        ws_instance->endpoint.send(it, message, websocketpp::frame::opcode::text, ec);
    }
}

void MockSimulationServer::handleCommand(const std::string& commandJson) {
    std::cout << "[WebSocket Server] Command Received: " << commandJson << std::endl;
}

std::string MockSimulationServer::formatUAVState(const std::string& id, const std::string& groupId, double lat, double lon, double alt, double heading, double battery, double startLat, double startLon, double targetLat, double targetLon) {
    std::stringstream ss;
    ss << "{\"type\": \"uav_update\", "
       << "\"id\": \"" << id << "\", "
       << "\"groupId\": \"" << groupId << "\", "
       << "\"lat\": " << std::fixed << std::setprecision(6) << lat << ", "
       << "\"lon\": " << lon << ", "
       << "\"alt\": " << alt << ", "
       << "\"heading\": " << heading << ", "
       << "\"battery\": " << battery << ", "
       << "\"startLat\": " << startLat << ", "
       << "\"startLon\": " << startLon << ", "
       << "\"targetLat\": " << targetLat << ", "
       << "\"targetLon\": " << targetLon << "}";
    return ss.str();
}

std::string MockSimulationServer::formatObstacleState(const std::string& type, const std::string& id, double lat, double lon, double rad, bool dynamic, const std::string& groupId) {
    std::stringstream ss;
    ss << "{\"type\": \"obstacle_update\", "
       << "\"id\": \"" << id << "\", "
       << "\"groupId\": \"" << groupId << "\", "
       << "\"lat\": " << std::fixed << std::setprecision(6) << lat << ", "
       << "\"lon\": " << lon << ", "
       << "\"radius\": " << rad << ", "
       << "\"dynamic\": " << (dynamic ? "true" : "false") << "}";
    return ss.str();
}

std::string SimulationServer::formatUAVState(const std::string& id, const std::string& groupId, double lat, double lon, double alt, double heading, double battery, double startLat, double startLon, double targetLat, double targetLon) {
    return MockSimulationServer::formatUAVState(id, groupId, lat, lon, alt, heading, battery, startLat, startLon, targetLat, targetLon);
}

std::string SimulationServer::formatObstacleState(const std::string& type, const std::string& id, double lat, double lon, double rad, bool dynamic, const std::string& groupId) {
    return MockSimulationServer::formatObstacleState(type, id, lat, lon, rad, dynamic, groupId);
}
