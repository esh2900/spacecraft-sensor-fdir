#pragma once
#include "SensorData.hpp"
#include <chrono>
#include <cmath>
#include <cstdint>
#include <map>
#include <optional>
#include <stdexcept>
#include <vector>

class SimulatedI2CBus {
    public:
        enum class DeviceType { IMU, GNSS };

    private:
    // Deterministic pseudo-random perturbation: no shared mutable RNG state.
    static double noise(std::uint8_t address, std::uint64_t sample, std::uint32_t axis) {
        std::uint32_t x = static_cast<std::uint32_t>(sample) * 0x9e3779b9u;
        x ^= static_cast<std::uint32_t>(address) * 0x85ebca6bu;
        x ^= axis * 0xc2b2ae35u;
        x ^= x >> 16;
        x *= 0x7feb352du;
        x ^= x >> 15;
        x *= 0x846ca68bu;
        x ^= x >> 16;
        return 2.0 * (static_cast<double>(x) / 4294967295.0) - 1.0;
    }

    struct Dropout {
        std::chrono::milliseconds begin;
        std::optional<std::chrono::milliseconds> end; // nullopt = permanent
    };
    struct Device {
        DeviceType type;
        std::vector<Dropout> dropouts;
    };

    bool isAvailable(std::uint8_t address, DeviceType expected,
                     Clock::time_point now) const {
        const auto it = devices_.find(address);
        if (it == devices_.end() || it->second.type != expected || now < startTime_)
            return false;
        const auto elapsed = now - startTime_;
        for (const auto& d : it->second.dropouts) {
            if (elapsed >= d.begin && (!d.end || elapsed < *d.end))
                return false;
        }
        return true;
    }

    Clock::time_point startTime_;
    std::map<std::uint8_t, Device> devices_;

public:
    explicit SimulatedI2CBus(Clock::time_point startTime) : startTime_(startTime) {}

    void registerSensor(std::uint8_t address, DeviceType type) {
        if (!devices_.emplace(address, Device{type, {}}).second)
            throw std::invalid_argument("Duplicate I2C address");
    }

    void addDropout(std::uint8_t address,
                    std::chrono::milliseconds begin,
                    std::optional<std::chrono::milliseconds> end = std::nullopt) {
        if (end && *end <= begin)
            throw std::invalid_argument("Invalid dropout interval");
        devices_.at(address).dropouts.push_back({begin, end});
    }

    bool readIMU(std::uint8_t address,
                 xyzvector& acceleration,
                 xyzvector& angularVelocity,
                 Clock::time_point now) const {
        if (!isAvailable(address, DeviceType::IMU, now)) return false;

        // Simulated IMU measurements:
        // Linear acceleration in m/s^2
        // Angular velocity in rad/s

        const double t = std::chrono::duration<double>(now - startTime_).count();
        const auto sample = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_).count() / 10);
        acceleration = {
            1.0 + 0.05 * std::sin(t) + 0.004 * noise(address, sample, 0),
            2.0 + 0.05 * std::cos(t) + 0.004 * noise(address, sample, 1),
            3.0 + 0.03 * std::sin(0.5 * t) + 0.004 * noise(address, sample, 2)
        };
        angularVelocity = {
            0.1 + 0.01 * std::sin(t) + 0.002 * noise(address, sample, 3),
            0.2 + 0.01 * std::cos(t) + 0.002 * noise(address, sample, 4),
            0.3 + 0.01 * std::sin(0.7 * t) + 0.002 * noise(address, sample, 5)
        };
        return true;
    }

    bool readGNSS(std::uint8_t address,
                  double& latitude, double& longitude, double& altitude,
                  Clock::time_point now) const {
        if (!isAvailable(address, DeviceType::GNSS, now)) return false;

        // Simulated GNSS measurements:
        // Latitude and longitude in decimal degrees (°)
        // Altitude in meters (m)

        const double t = std::chrono::duration<double>(now - startTime_).count();
        const auto sample = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_).count() / 50);
        latitude = 50.0 + 0.00001 * std::sin(0.2 * t) + 0.000001 * noise(address, sample, 0);
        longitude = 10.0 + 0.00001 * std::cos(0.2 * t) + 0.000001 * noise(address, sample, 1);
        altitude = 100.0 + 0.2 * std::sin(0.2 * t) + 0.05 * noise(address, sample, 2);
        return true;

    }

};
