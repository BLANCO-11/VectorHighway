#pragma once

#include "Types.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
class TenantManager {
public:
    struct Tenant {
        std::string id;
        std::string name;
        std::vector<std::string> drones;
        std::vector<std::string> operators;
    };

    TenantManager() = default;

    void addTenant(const Tenant& tenant);
    void removeTenant(const std::string& tenantId);
    Tenant* getTenant(const std::string& tenantId);
    const std::vector<Tenant>& getAllTenants() const;

    bool isDroneOwnedByTenant(const std::string& droneId, const std::string& tenantId) const;
    std::vector<std::string> getTenantDrones(const std::string& tenantId) const;
    int getTotalDroneCount() const;

    std::function<void(const std::string& tenantId, const std::string& event)> onEvent;

private:
    std::vector<Tenant> tenants;
    std::map<std::string, size_t> tenantIndex;
};
