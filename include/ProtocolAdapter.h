#pragma once

#include <string>
#include <functional>

struct ExternalTelemetry;
struct CommandEvent;

class ProtocolAdapter {
public:
    virtual ~ProtocolAdapter() = default;

    virtual bool connect(const std::string& endpoint) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    virtual ExternalTelemetry translateTelemetry(const std::string& rawMessage) = 0;
    virtual std::string translateCommand(const CommandEvent& cmd) = 0;

    virtual std::string adapterName() const = 0;
    virtual std::string protocolVersion() const = 0;

    std::function<void(const ExternalTelemetry&)> onTelemetry;
    std::function<void(const std::string& log)> onLog;
};
