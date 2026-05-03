#include "../include/ProtocolAdapter.h"
#include "../include/CommandEvent.h"
#include <iostream>
#include <sstream>

class MAVLinkAdapter : public ProtocolAdapter {
public:
    MAVLinkAdapter() = default;

    bool connect(const std::string& endpoint) override {
        std::cout << "[MAVLink] Connecting to " << endpoint << std::endl;
        connected = true;
        return true;
    }

    void disconnect() override {
        std::cout << "[MAVLink] Disconnecting" << std::endl;
        connected = false;
    }

    bool isConnected() const override {
        return connected;
    }

    ExternalTelemetry translateTelemetry(const std::string& rawMessage) override {
        ExternalTelemetry ext;
        try {
            auto j = json::parse(rawMessage);
            if (j.contains("id")) ext.id = j["id"].get<std::string>();
            if (j.contains("lat")) ext.lat = j["lat"].get<double>();
            if (j.contains("lon")) ext.lon = j["lon"].get<double>();
            if (j.contains("alt")) ext.alt = j["alt"].get<double>();
            if (j.contains("heading")) ext.heading = j["heading"].get<double>();
            if (j.contains("battery")) ext.battery = j["battery"].get<double>();
            if (j.contains("speed")) ext.speed = j["speed"].get<double>();
            if (j.contains("status")) ext.status = j["status"].get<std::string>();
        } catch (...) {
            std::cerr << "[MAVLink] Failed to parse telemetry" << std::endl;
        }
        return ext;
    }

    std::string translateCommand(const CommandEvent& cmd) override {
        json j;
        j["type"] = cmd.type;
        j["target"] = cmd.targetId;
        if (cmd.lat != 0.0 || cmd.lon != 0.0) {
            j["lat"] = cmd.lat;
            j["lon"] = cmd.lon;
        }
        return j.dump();
    }

    std::string adapterName() const override {
        return "MAVLink v2";
    }

    std::string protocolVersion() const override {
        return "2.0";
    }

private:
    bool connected = false;
};
