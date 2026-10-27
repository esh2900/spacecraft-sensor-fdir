#pragma once

#include<chrono>
#include <cstdint>

using Clock = std::chrono::steady_clock;

struct xyzvector{
    double x;
    double y;
    double z;
};

// Sensor Input Data
struct IMUData{
    // Identification
    std::uint8_t sensorID;
    std::uint64_t measurementID;
    Clock::time_point timestamp;
    // Data
    xyzvector acceleration; // m/s²
    xyzvector angularVelocity; // rad/s
    // Validity
    bool measurementValid;

};

struct GNSSData{
    // Identification
    std::uint8_t sensorID;
    std::uint64_t measurementID;
    Clock::time_point timestamp;
    // Data
    double latitude;  // degrees
    double longitude; // degrees
    double altitude;  // meters
    // Validity
    bool fixValid;
};

// Subsystem output data
struct GNSSOutput
{
    double latitude;
    double longitude;
    double altitude;
    Clock::time_point timestamp;

};

struct IMUOutput
{
    xyzvector acceleration;  // m/s²
    xyzvector angularVelocity; // rad/s
    Clock::time_point timestamp;
};