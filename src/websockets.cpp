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

using json = nlohmann::json;

typedef websocketpp::server<websocketpp::config::asio> server;

// Global thread-safe state to pass data to the main simulation thread
std::mutex g_ui_simMtx;
bool g_ui_targetUpdated = false;
double g_ui_targetLat = 0.0;
double g_ui_targetLon = 0.0;

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
                if (j.contains("type")) {
                    std::string type = j["type"];
                    // Check if the user clicked a new target on the Globe
                    if (type == "set_target") {
                        std::lock_guard<std::mutex> lock(g_ui_simMtx);
                        g_ui_targetLat = j["lat"];
                        g_ui_targetLon = j["lon"];
                        g_ui_targetUpdated = true;
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

std::string MockSimulationServer::formatUAVState(double lat, double lon, double alt, double heading, double battery, double startLat, double startLon, double targetLat, double targetLon) {
    std::stringstream ss;
    ss << "{\"type\": \"uav_update\", "
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

std::string MockSimulationServer::formatObstacleState(const std::string& type, const std::string& id, double lat, double lon, double rad, bool dynamic) {
    std::stringstream ss;
    ss << "{\"type\": \"obstacle_update\", "
       << "\"id\": \"" << id << "\", "
       << "\"lat\": " << std::fixed << std::setprecision(6) << lat << ", "
       << "\"lon\": " << lon << ", "
       << "\"radius\": " << rad << ", "
       << "\"dynamic\": " << (dynamic ? "true" : "false") << "}";
    return ss.str();
}

std::string SimulationServer::formatUAVState(double lat, double lon, double alt, double heading, double battery, double startLat, double startLon, double targetLat, double targetLon) {
    return MockSimulationServer::formatUAVState(lat, lon, alt, heading, battery, startLat, startLon, targetLat, targetLon);
}

std::string SimulationServer::formatObstacleState(const std::string& type, const std::string& id, double lat, double lon, double rad, bool dynamic) {
    return MockSimulationServer::formatObstacleState(type, id, lat, lon, rad, dynamic);
}
