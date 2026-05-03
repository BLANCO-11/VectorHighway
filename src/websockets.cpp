#include "../include/websockets.h"
#include "../include/SimulationContext.h"
#include "../include/CommandEvent.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <set>
#include <memory>
#include <chrono>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include "../include/Environment.h"

using json = nlohmann::json;

typedef websocketpp::server<websocketpp::config::asio> server;

class WSServerImpl {
public:
    server endpoint;
    std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> connections;
    std::mutex mtx;
    std::thread server_thread;
    SimulationContext* context = nullptr;

    std::chrono::steady_clock::time_point rateLimitWindowStart;
    int rateLimitMessageCount = 0;
    static constexpr int MAX_MSGS_PER_SEC = 100;

    WSServerImpl(SimulationContext* ctx) : context(ctx), rateLimitWindowStart(std::chrono::steady_clock::now()) {
        endpoint.clear_access_channels(websocketpp::log::alevel::all);
        endpoint.init_asio();

        endpoint.set_open_handler([this](websocketpp::connection_hdl hdl) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                connections.insert(hdl);
            }
            std::cout << "[WebSocket] Client connected." << std::endl;
            broadcastConnectionStatus(true);
        });

        endpoint.set_close_handler([this](websocketpp::connection_hdl hdl) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                connections.erase(hdl);
            }
            std::cout << "[WebSocket] Client disconnected." << std::endl;
        });

        endpoint.set_message_handler([this](websocketpp::connection_hdl, server::message_ptr msg) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - rateLimitWindowStart).count();

            if (elapsed >= 1000) {
                rateLimitWindowStart = now;
                rateLimitMessageCount = 0;
            }

            rateLimitMessageCount++;
            if (rateLimitMessageCount > MAX_MSGS_PER_SEC) {
                std::cout << "[WebSocket] Rate limit exceeded — dropping message" << std::endl;
                return;
            }

            std::string payload = msg->get_payload();
            try {
                auto j = json::parse(payload);
                if (j.contains("topic")) {
                    std::string topic = j["topic"].get<std::string>();
                    auto msgPayload = j["payload"];

                    if (topic.find("cmd/fleet/") == 0 && topic.find("/target") != std::string::npos) {
                        size_t firstSlash = topic.find('/', 10);
                        std::string id = topic.substr(10, firstSlash - 10);
                        CommandEvent cmd;
                        cmd.type = "target";
                        cmd.targetId = id;
                        cmd.lat = msgPayload["lat"].get<double>();
                        cmd.lon = msgPayload["lon"].get<double>();
                        context->pushCommand(cmd);
                    }
                    else if (topic.find("cmd/fleet/") == 0 && topic.find("/control") != std::string::npos) {
                        size_t firstSlash = topic.find('/', 10);
                        std::string id = topic.substr(10, firstSlash - 10);
                        CommandEvent cmd;
                        cmd.type = "control";
                        cmd.targetId = id;
                        cmd.speed = msgPayload.contains("speed") ? msgPayload["speed"].get<double>() : -1;
                        cmd.drain = msgPayload.contains("batteryDrain") ? msgPayload["batteryDrain"].get<double>() : -1;
                        context->pushCommand(cmd);
                    }
                    else if (topic == "cmd/environment/obstacle") {
                        CommandEvent cmd;
                        cmd.type = "obstacle";
                        cmd.lat = msgPayload["lat"].get<double>();
                        cmd.lon = msgPayload["lon"].get<double>();
                        cmd.radius = msgPayload.contains("radius") ? msgPayload["radius"].get<double>() : 2.0;
                        cmd.groupId = msgPayload.contains("groupId") ? msgPayload["groupId"].get<std::string>() : "";
                        context->pushCommand(cmd);
                    }
                    else if (topic == "cmd/fleet/spawn") {
                        CommandEvent cmd;
                        cmd.type = "spawn";
                        cmd.lat = msgPayload["lat"].get<double>();
                        cmd.lon = msgPayload["lon"].get<double>();
                        cmd.groupId = msgPayload.contains("groupId") ? msgPayload["groupId"].get<std::string>() : "alpha";
                        context->pushCommand(cmd);
                    }
                    else if (topic.find("telemetry/external/") == 0) {
                        std::string id = topic.substr(19);
                        ExternalTelemetry ext;
                        ext.id = id;
                        ext.groupId = msgPayload.contains("groupId") ? msgPayload["groupId"].get<std::string>() : "alpha";
                        ext.lat = msgPayload["lat"].get<double>();
                        ext.lon = msgPayload["lon"].get<double>();
                        ext.alt = msgPayload.contains("alt") ? msgPayload["alt"].get<double>() : 0.0;
                        ext.heading = msgPayload.contains("heading") ? msgPayload["heading"].get<double>() : 0.0;
                        ext.battery = msgPayload.contains("battery") ? msgPayload["battery"].get<double>() : 100.0;
                        ext.speed = msgPayload.contains("speed") ? msgPayload["speed"].get<double>() : 0.0;
                        ext.status = msgPayload.contains("status") ? msgPayload["status"].get<std::string>() : "FLYING";
                        context->updateExternalDrone(ext);
                    }
                    else if (topic.find("cmd/mission/") == 0 && topic.find("/upload") != std::string::npos) {
                        size_t start = 12;
                        size_t uploadPos = topic.find("/upload");
                        std::string id = topic.substr(start, uploadPos - start);
                        CommandEvent cmd;
                        cmd.type = "mission_upload";
                        cmd.targetId = id;
                        cmd.missionJson = msgPayload.dump();
                        context->pushCommand(cmd);
                    }
                    else if (topic.find("cmd/mission/") == 0 && topic.find("/start") != std::string::npos) {
                        size_t start = 12;
                        size_t endPos = topic.find("/start");
                        std::string id = topic.substr(start, endPos - start);
                        CommandEvent cmd;
                        cmd.type = "mission_start";
                        cmd.targetId = id;
                        context->pushCommand(cmd);
                    }
                    else if (topic.find("cmd/mission/") == 0 && topic.find("/rth") != std::string::npos) {
                        size_t start = 12;
                        size_t endPos = topic.find("/rth");
                        std::string id = topic.substr(start, endPos - start);
                        CommandEvent cmd;
                        cmd.type = "mission_rth";
                        cmd.targetId = id;
                        context->pushCommand(cmd);
                    }
                    else if (topic == "cmd/system/recording") {
                        CommandEvent cmd;
                        cmd.type = "recording";
                        cmd.speed = msgPayload.contains("enabled") && msgPayload["enabled"].get<bool>() ? 1.0 : 0.0;
                        context->pushCommand(cmd);
                    }
                    else if (topic == "cmd/system/replay") {
                        CommandEvent cmd;
                        cmd.type = "replay";
                        cmd.missionJson = msgPayload.dump();
                        context->pushCommand(cmd);
                    }
                    else if (topic.find("cmd/mission/") == 0 && topic.find("/edit") != std::string::npos) {
                        size_t start = 12;
                        size_t endPos = topic.find("/edit");
                        std::string id = topic.substr(start, endPos - start);
                        CommandEvent cmd;
                        cmd.type = "mission_edit";
                        cmd.targetId = id;
                        cmd.missionJson = msgPayload.dump();
                        context->pushCommand(cmd);
                    }
                    else if (topic.find("cmd/environment/noflyzone") == 0) {
                        CommandEvent cmd;
                        cmd.type = "noflyzone";
                        cmd.targetId = msgPayload.contains("name") ? msgPayload["name"].get<std::string>() : "";
                        cmd.lat = msgPayload.contains("lat") ? msgPayload["lat"].get<double>() : 0.0;
                        cmd.lon = msgPayload.contains("lon") ? msgPayload["lon"].get<double>() : 0.0;
                        cmd.radius = msgPayload.contains("radius") ? msgPayload["radius"].get<double>() : 5.0;
                        context->pushCommand(cmd);
                    }
                    else if (topic.find("cmd/system/checklist") == 0) {
                        CommandEvent cmd;
                        cmd.type = "checklist_override";
                        cmd.missionJson = msgPayload.dump();
                        context->pushCommand(cmd);
                    }
                }
            } catch (const std::exception& e) {
                std::cout << "[WebSocket] JSON Parse Error: " << e.what() << std::endl;
            }
        });
    }

    void broadcastConnectionStatus(bool connected) {
        json status;
        status["type"] = "connection_status";
        status["connected"] = connected;
        status["message"] = connected
            ? "Connected to UAV Engine v2.0"
            : "Client disconnected";

        std::lock_guard<std::mutex> lock(mtx);
        for (auto it = connections.begin(); it != connections.end(); ) {
            websocketpp::lib::error_code ec;
            endpoint.send(*it, status.dump(), websocketpp::frame::opcode::text, ec);
            if (ec) {
                it = connections.erase(it);
            } else {
                ++it;
            }
        }
    }
};

static std::unique_ptr<WSServerImpl> ws_instance;

bool MockSimulationServer::start(int port, SimulationContext* context) {
    std::cout << "[WebSocket Server] Starting on port " << port << "..." << std::endl;
    ws_instance = std::make_unique<WSServerImpl>(context);

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
        ws_instance.reset();
        std::cout << "[WebSocket Server] Stopped." << std::endl;
    }
}

void MockSimulationServer::broadcast(const std::string& message) {
    if (!ws_instance) return;

    std::lock_guard<std::mutex> lock(ws_instance->mtx);
    for (auto it = ws_instance->connections.begin(); it != ws_instance->connections.end(); ) {
        websocketpp::lib::error_code ec;
        ws_instance->endpoint.send(*it, message, websocketpp::frame::opcode::text, ec);
        if (ec) {
            it = ws_instance->connections.erase(it);
        } else {
            ++it;
        }
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
       << "\"speed\": 0.25, "
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
