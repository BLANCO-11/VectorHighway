#include "../include/AuthManager.h"
#include <algorithm>
#include <iostream>

const std::map<std::string, OperatorRole> AuthManager::ROLE_MAP = {
    {"COMMANDER", OperatorRole::COMMANDER},
    {"SENSOR_OP", OperatorRole::SENSOR_OP},
    {"SUPERVISOR", OperatorRole::SUPERVISOR},
    {"VIEWER", OperatorRole::VIEWER},
};

const std::map<OperatorRole, std::vector<std::string>> AuthManager::DEFAULT_PERMISSIONS = {
    {OperatorRole::COMMANDER, {"all"}},
    {OperatorRole::SENSOR_OP, {"view_telemetry", "control_gimbal", "view_missions"}},
    {OperatorRole::SUPERVISOR, {"all", "override"}},
    {OperatorRole::VIEWER, {"view_telemetry"}},
};

bool AuthManager::authenticate(const std::string& token, Operator& outOp) const {
    if (tokenValidator) {
        return tokenValidator(token, outOp);
    }

    auto it = operators.find(token);
    if (it != operators.end()) {
        outOp = it->second;
        return true;
    }
    return false;
}

bool AuthManager::hasPermission(const Operator& op, const std::string& permission) const {
    const auto& perms = DEFAULT_PERMISSIONS.at(op.role);
    if (std::find(perms.begin(), perms.end(), "all") != perms.end()) return true;
    return std::find(perms.begin(), perms.end(), permission) != perms.end();
}

bool AuthManager::canExecuteCommand(const Operator& op, const std::string& commandTopic) const {
    if (op.role == OperatorRole::SUPERVISOR) return true;

    if (op.role == OperatorRole::VIEWER) {
        if (commandTopic.find("cmd/") == 0) return false;
    }

    if (op.role == OperatorRole::SENSOR_OP) {
        if (commandTopic.find("cmd/mission/") == 0) return false;
        if (commandTopic.find("cmd/fleet/") == 0) return false;
    }

    return true;
}

void AuthManager::registerOperator(const Operator& op) {
    operators[op.id] = op;
}

void AuthManager::removeOperator(const std::string& opId) {
    operators.erase(opId);
}

std::vector<Operator> AuthManager::getConnectedOperators() const {
    std::vector<Operator> ops;
    for (const auto& [_, op] : operators) {
        ops.push_back(op);
    }
    return ops;
}

std::vector<std::string> AuthManager::getPermissionsForRole(OperatorRole role) {
    auto it = DEFAULT_PERMISSIONS.find(role);
    if (it != DEFAULT_PERMISSIONS.end()) return it->second;
    return {};
}

void AuthManager::setTokenValidator(std::function<bool(const std::string&, Operator&)> validator) {
    tokenValidator = validator;
}
