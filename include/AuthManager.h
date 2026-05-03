#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

enum class OperatorRole {
    COMMANDER,
    SENSOR_OP,
    SUPERVISOR,
    VIEWER
};

struct Operator {
    std::string id;
    std::string name;
    OperatorRole role = OperatorRole::VIEWER;
    std::vector<std::string> permissions;
    double connectedAt = 0.0;
};

class AuthManager {
public:
    AuthManager() = default;

    bool authenticate(const std::string& token, Operator& outOp) const;
    bool hasPermission(const Operator& op, const std::string& permission) const;
    bool canExecuteCommand(const Operator& op, const std::string& commandTopic) const;

    void registerOperator(const Operator& op);
    void removeOperator(const std::string& opId);
    std::vector<Operator> getConnectedOperators() const;

    static std::vector<std::string> getPermissionsForRole(OperatorRole role);

    void setTokenValidator(std::function<bool(const std::string&, Operator&)> validator);

private:
    std::map<std::string, Operator> operators;
    std::function<bool(const std::string&, Operator&)> tokenValidator;

    static const std::map<std::string, OperatorRole> ROLE_MAP;
    static const std::map<OperatorRole, std::vector<std::string>> DEFAULT_PERMISSIONS;
};
