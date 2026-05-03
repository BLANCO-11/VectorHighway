#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>

struct WeatherData {
    double windSpeed = 0.0;
    double windDirection = 0.0;
    double precipitation = 0.0;
    double visibility = 10.0;
    double temperature = 15.0;
    double timestamp = 0.0;
};

struct WeatherCell {
    double lat, lon;
    double windSpeed, windDirection;
    double precipitation;
};

class WeatherAPI {
public:
    WeatherAPI() = default;

    WeatherData fetchAt(double lat, double lon) const;
    std::vector<WeatherCell> fetchForBoundingBox(double latMin, double latMax,
                                                  double lonMin, double lonMax) const;

    void setApiKey(const std::string& key) { apiKey = key; }
    void setCacheTTL(int seconds) { cacheTTL = seconds; }

    std::function<WeatherData(double, double)> onFetch;

private:
    std::string apiKey;
    int cacheTTL = 300;
    mutable std::map<std::pair<int, int>, std::pair<WeatherData, double>> cache;

    WeatherData fetchFromNOAA(double lat, double lon) const;
    WeatherData fetchFromOpenWeatherMap(double lat, double lon) const;
};

struct TerrainData {
    double elevation = 0.0;
    double slope = 0.0;
    double aspect = 0.0;
};

class TerrainLoader {
public:
    TerrainLoader() = default;

    TerrainData getElevation(double lat, double lon) const;
    std::vector<TerrainData> getElevationProfile(const std::vector<std::pair<double, double>>& points) const;

    bool loadSRTMFile(const std::string& filename);

    std::function<TerrainData(double, double)> onQuery;

private:
    mutable std::map<std::pair<int, int>, double> elevationCache;

    double interpolateElevation(double lat, double lon) const;
    int srtxTileX(double lon) const;
    int srtmTileY(double lat) const;
};

struct AirspaceZone {
    std::string id;
    std::string name;
    std::string airspaceClass;
    std::vector<std::pair<double, double>> polygon;
    double lowerLimit = 0.0;
    double upperLimit = 99999.0;
    bool active = true;
};

class AirspaceParser {
public:
    AirspaceParser() = default;

    std::vector<AirspaceZone> parseOpenAir(const std::string& openAirContent) const;
    std::vector<AirspaceZone> parseTFR(const std::string& tfrJson) const;

    bool isInAirspace(double lat, double lon, double alt, const AirspaceZone& zone) const;
    std::vector<AirspaceZone> findActiveZones(double lat, double lon, double alt,
                                               const std::vector<AirspaceZone>& zones) const;

    void loadDefaultZones();
    const std::vector<AirspaceZone>& getDefaultZones() const;

private:
    std::vector<AirspaceZone> defaultZones;

    AirspaceZone parseOpenAirSegment(const std::vector<std::string>& lines, size_t& idx) const;
};
