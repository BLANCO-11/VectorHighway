#pragma once

#include "Types.h"
#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <chrono>
#include <memory>

struct LogEntry {
    double timestamp;
    std::string droneId;
    double lat;
    double lon;
    double alt;
    double heading;
    double speed;
    double battery;
    std::string missionState;
};

class FlightRecorder {
public:
    FlightRecorder() = default;
    ~FlightRecorder();

    bool startRecording(const std::string& directory = "./");
    void stopRecording();
    bool isRecording() const;

    void writeEntry(const LogEntry& entry);
    void writeEntry(double timestamp, const std::string& droneId,
                    double lat, double lon, double alt,
                    double heading, double speed, double battery,
                    const std::string& missionState);

    std::vector<LogEntry> loadLog(const std::string& filename) const;

    std::vector<std::string> listAvailableLogs(const std::string& directory = "./") const;

    void rotateFile();

    std::string getCurrentFilename() const;

    static constexpr double MAX_FILE_SIZE_MB = 100.0;
    static constexpr double MAX_RECORDING_HOURS = 1.0;

private:
    std::unique_ptr<std::ofstream> fileStream;
    std::string currentFilename;
    std::string logDirectory;
    bool recording = false;
    mutable std::mutex mtx;
    std::chrono::steady_clock::time_point recordingStartTime;

    std::string generateFilename() const;
    double getFileSizeMB() const;
    bool shouldRotate() const;
    std::string entryToJson(const LogEntry& entry) const;
    LogEntry jsonToEntry(const std::string& json) const;
};
