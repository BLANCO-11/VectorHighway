#include "../include/TenantManager.h"
#include <algorithm>

void TenantManager::addTenant(const Tenant& tenant) {
    tenantIndex[tenant.id] = tenants.size();
    tenants.push_back(tenant);
    if (onEvent) onEvent(tenant.id, "created");
}

void TenantManager::removeTenant(const std::string& tenantId) {
    auto it = tenantIndex.find(tenantId);
    if (it != tenantIndex.end()) {
        tenants.erase(tenants.begin() + it->second);
        tenantIndex.erase(it);
        for (size_t i = 0; i < tenants.size(); ++i) {
            tenantIndex[tenants[i].id] = i;
        }
        if (onEvent) onEvent(tenantId, "removed");
    }
}

TenantManager::Tenant* TenantManager::getTenant(const std::string& tenantId) {
    auto it = tenantIndex.find(tenantId);
    if (it != tenantIndex.end()) return &tenants[it->second];
    return nullptr;
}

const std::vector<TenantManager::Tenant>& TenantManager::getAllTenants() const {
    return tenants;
}

bool TenantManager::isDroneOwnedByTenant(const std::string& droneId, const std::string& tenantId) const {
    auto it = tenantIndex.find(tenantId);
    if (it == tenantIndex.end()) return false;
    const auto& droneList = tenants[it->second].drones;
    return std::find(droneList.begin(), droneList.end(), droneId) != droneList.end();
}

std::vector<std::string> TenantManager::getTenantDrones(const std::string& tenantId) const {
    auto it = tenantIndex.find(tenantId);
    if (it != tenantIndex.end()) return tenants[it->second].drones;
    return {};
}

int TenantManager::getTotalDroneCount() const {
    int count = 0;
    for (const auto& t : tenants) count += t.drones.size();
    return count;
}
