#include "../include/WeatherAPI.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

WeatherData WeatherAPI::fetchAt(double lat, double lon) const {
    std::pair<int, int> key(static_cast<int>(lat * 10), static_cast<int>(lon * 10));
    auto it = cache.find(key);
    if (it != cache.end()) {
        return it->second.first;
    }

    WeatherData data;
    if (onFetch) {
        data = onFetch(lat, lon);
    } else {
        data.windSpeed = 5.0 + std::sin(lat * 0.1) * 3.0;
        data.windDirection = std::fmod(lon * 0.5, 360.0);
        data.precipitation = std::max(0.0, std::sin(lat * 0.05) * std::cos(lon * 0.03) * 50.0);
        data.visibility = 10.0 + std::sin(lat * 0.02) * 5.0;
        data.temperature = 20.0 - std::abs(lat) * 0.3;
    }
    data.timestamp = static_cast<double>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());

    cache[key] = {data, data.timestamp};
    return data;
}

std::vector<WeatherCell> WeatherAPI::fetchForBoundingBox(double latMin, double latMax,
                                                          double lonMin, double lonMax) const {
    std::vector<WeatherCell> cells;
    double step = 1.0;
    for (double lat = latMin; lat <= latMax; lat += step) {
        for (double lon = lonMin; lon <= lonMax; lon += step) {
            auto wd = fetchAt(lat, lon);
            cells.push_back({lat, lon, wd.windSpeed, wd.windDirection, wd.precipitation});
        }
    }
    return cells;
}

TerrainData TerrainLoader::getElevation(double lat, double lon) const {
    std::pair<int, int> key(static_cast<int>(lat), static_cast<int>(lon));
    auto it = elevationCache.find(key);
    if (it != elevationCache.end()) {
        return {it->second, 0.0, 0.0};
    }

    if (onQuery) return onQuery(lat, lon);

    double elev = std::max(0.0, 500.0 * std::sin(lat * 0.05) * std::cos(lon * 0.03) + 200.0);
    elevationCache[key] = elev;
    return {elev, 0.0, 0.0};
}

std::vector<TerrainData> TerrainLoader::getElevationProfile(
    const std::vector<std::pair<double, double>>& points) const {
    std::vector<TerrainData> profile;
    for (const auto& [lat, lon] : points) {
        profile.push_back(getElevation(lat, lon));
    }
    return profile;
}

bool TerrainLoader::loadSRTMFile(const std::string& filename) {
    std::cout << "[Terrain] SRTM loading not implemented: " << filename << std::endl;
    return false;
}

AirspaceZone AirspaceParser::parseOpenAirSegment(const std::vector<std::string>& lines, size_t& idx) const {
    AirspaceZone zone;
    zone.id = "zone_" + std::to_string(idx);
    if (idx < lines.size()) {
        zone.name = lines[idx];
    }
    idx++;
    while (idx < lines.size() && lines[idx].find("AC ") != 0) {
        if (lines[idx].find("AN ") == 0) {
            zone.name = lines[idx].substr(3);
        }
        if (lines[idx].find("AL ") == 0) {
            zone.airspaceClass = lines[idx].substr(3);
        }
        idx++;
    }
    zone.active = true;
    return zone;
}

std::vector<AirspaceZone> AirspaceParser::parseOpenAir(const std::string& content) const {
    std::vector<AirspaceZone> zones;
    std::vector<std::string> lines;
    std::istringstream ss(content);
    std::string line;
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }

    size_t idx = 0;
    while (idx < lines.size()) {
        if (lines[idx].find("AC ") == 0) {
            zones.push_back(parseOpenAirSegment(lines, idx));
        } else {
            ++idx;
        }
    }

    return zones;
}

std::vector<AirspaceZone> AirspaceParser::parseTFR(const std::string& tfrJson) const {
    std::vector<AirspaceZone> zones;
    try {
        auto j = json::parse(tfrJson);
        if (j.contains("features")) {
            for (const auto& feature : j["features"]) {
                AirspaceZone zone;
                zone.id = feature.contains("id") ? feature["id"].get<std::string>() : "";
                zone.name = feature.contains("properties") && feature["properties"].contains("name")
                    ? feature["properties"]["name"].get<std::string>() : "";
                zone.airspaceClass = "TFR";
                zones.push_back(zone);
            }
        }
    } catch (...) {
        std::cerr << "[Airspace] Failed to parse TFR JSON" << std::endl;
    }
    return zones;
}

bool AirspaceParser::isInAirspace(double lat, double lon, double alt,
                                   const AirspaceZone& zone) const {
    if (alt < zone.lowerLimit || alt > zone.upperLimit) return false;
    if (zone.polygon.size() < 3) return false;
    bool inside = false;
    size_t j = zone.polygon.size() - 1;
    for (size_t i = 0; i < zone.polygon.size(); ++i) {
        if ((zone.polygon[i].second > lon) != (zone.polygon[j].second > lon) &&
            lat < ((zone.polygon[j].first - zone.polygon[i].first) *
            (lon - zone.polygon[i].second) / (zone.polygon[j].second - zone.polygon[i].second) +
            zone.polygon[i].first)) {
            inside = !inside;
        }
        j = i;
    }
    return inside;
}

std::vector<AirspaceZone> AirspaceParser::findActiveZones(double lat, double lon, double alt,
                                                           const std::vector<AirspaceZone>& zones) const {
    std::vector<AirspaceZone> active;
    for (const auto& zone : zones) {
        if (zone.active && isInAirspace(lat, lon, alt, zone)) {
            active.push_back(zone);
        }
    }
    return active;
}

void AirspaceParser::loadDefaultZones() {
    defaultZones.clear();
    AirspaceZone classA;
    classA.id = "class_a_default";
    classA.name = "Class A Airspace";
    classA.airspaceClass = "A";
    classA.lowerLimit = 18000;
    defaultZones.push_back(classA);
}

const std::vector<AirspaceZone>& AirspaceParser::getDefaultZones() const {
    return defaultZones;
}
