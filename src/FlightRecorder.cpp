#include "../include/FlightRecorder.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

FlightRecorder::~FlightRecorder() {
    stopRecording();
}

bool FlightRecorder::startRecording(const std::string& directory) {
    std::lock_guard<std::mutex> lock(mtx);
    if (recording) return false;

    logDirectory = directory;

    namespace fs = std::filesystem;
    if (!fs::exists(logDirectory)) {
        fs::create_directories(logDirectory);
    }

    currentFilename = generateFilename();
    fileStream = std::make_unique<std::ofstream>(currentFilename);
    if (!fileStream->is_open()) {
        std::cerr << "[FlightRecorder] Failed to open: " << currentFilename << std::endl;
        return false;
    }

    recording = true;
    recordingStartTime = std::chrono::steady_clock::now();
    std::cout << "[FlightRecorder] Recording started: " << currentFilename << std::endl;
    return true;
}

void FlightRecorder::stopRecording() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!recording) return;

    if (fileStream && fileStream->is_open()) {
        fileStream->close();
    }
    recording = false;
    std::cout << "[FlightRecorder] Recording stopped." << std::endl;
}

bool FlightRecorder::isRecording() const {
    return recording;
}

void FlightRecorder::writeEntry(const LogEntry& entry) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!recording || !fileStream || !fileStream->is_open()) return;

    if (shouldRotate()) {
        fileStream->close();
        currentFilename = generateFilename();
        fileStream = std::make_unique<std::ofstream>(currentFilename);
    }

    *fileStream << entryToJson(entry) << std::endl;
}

void FlightRecorder::writeEntry(double timestamp, const std::string& droneId,
                                 double lat, double lon, double alt,
                                 double heading, double speed, double battery,
                                 const std::string& missionState) {
    LogEntry entry{timestamp, droneId, lat, lon, alt, heading, speed, battery, missionState};
    writeEntry(entry);
}

std::vector<LogEntry> FlightRecorder::loadLog(const std::string& filename) const {
    std::vector<LogEntry> entries;
    std::ifstream file(filename);
    if (!file.is_open()) return entries;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        try {
            entries.push_back(jsonToEntry(line));
        } catch (...) {
            std::cerr << "[FlightRecorder] Skipping malformed line in " << filename << std::endl;
        }
    }
    return entries;
}

std::vector<std::string> FlightRecorder::listAvailableLogs(const std::string& directory) const {
    std::vector<std::string> logs;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            auto name = entry.path().filename().string();
            if (name.find("flightlog_") == 0 && name.find(".jsonl") != std::string::npos) {
                logs.push_back(entry.path().string());
            }
        }
    } catch (...) {}
    std::sort(logs.begin(), logs.end());
    return logs;
}

void FlightRecorder::rotateFile() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!recording || !fileStream) return;

    fileStream->close();
    currentFilename = generateFilename();
    fileStream = std::make_unique<std::ofstream>(currentFilename);
    recordingStartTime = std::chrono::steady_clock::now();
}

std::string FlightRecorder::getCurrentFilename() const {
    return currentFilename;
}

std::string FlightRecorder::generateFilename() const {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_r(&t, &tm);
    std::stringstream ss;
    ss << logDirectory << "/flightlog_"
       << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".jsonl";
    return ss.str();
}

double FlightRecorder::getFileSizeMB() const {
    if (!fileStream || !fileStream->is_open()) return 0.0;
    auto pos = fileStream->tellp();
    return static_cast<double>(pos) / (1024.0 * 1024.0);
}

bool FlightRecorder::shouldRotate() const {
    if (getFileSizeMB() >= MAX_FILE_SIZE_MB) return true;
    auto elapsed = std::chrono::steady_clock::now() - recordingStartTime;
    return std::chrono::duration_cast<std::chrono::hours>(elapsed).count() >= 1;
}

std::string FlightRecorder::entryToJson(const LogEntry& entry) const {
    json j;
    j["timestamp"] = entry.timestamp;
    j["droneId"] = entry.droneId;
    j["lat"] = entry.lat;
    j["lon"] = entry.lon;
    j["alt"] = entry.alt;
    j["heading"] = entry.heading;
    j["speed"] = entry.speed;
    j["battery"] = entry.battery;
    j["missionState"] = entry.missionState;
    return j.dump();
}

LogEntry FlightRecorder::jsonToEntry(const std::string& jsonStr) const {
    auto j = json::parse(jsonStr);
    LogEntry entry;
    entry.timestamp = j["timestamp"].get<double>();
    entry.droneId = j["droneId"].get<std::string>();
    entry.lat = j["lat"].get<double>();
    entry.lon = j["lon"].get<double>();
    entry.alt = j["alt"].get<double>();
    entry.heading = j["heading"].get<double>();
    entry.speed = j["speed"].get<double>();
    entry.battery = j["battery"].get<double>();
    entry.missionState = j["missionState"].get<std::string>();
    return entry;
}
