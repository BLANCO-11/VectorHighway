#pragma once

#include <string>
#include <vector>
#include <chrono>

using DroneID = std::string;
using GroupID = std::string;

constexpr double EARTH_RADIUS_KM = 6371.0;
constexpr double SIMULATION_TICK_HZ = 20.0;
constexpr double SIMULATION_TICK_S = 1.0 / SIMULATION_TICK_HZ;
constexpr int DEFAULT_PORT = 7200;
constexpr double STALE_DRONE_TIMEOUT_S = 30.0;
constexpr double BATTERY_MAX = 100.0;