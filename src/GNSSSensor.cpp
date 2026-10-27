#include <chrono>
#include <atomic>
#include <thread>

#include "GNSSSensor.hpp"
#include "SensorData.hpp"

// Simulation
#include "SimulatedI2CBus.hpp"

using Clock = std::chrono::steady_clock;

void GNSSSensor::run(const std::atomic<bool>& running, Clock::time_point startTime)
{
    
    const auto period = std::chrono::nanoseconds(1'000'000'000LL / acquisitionRateHz_);
    auto nextTick = startTime;

    while (running.load())
    {    
        // Sleep until next scheduled acquisition
        std::this_thread::sleep_until(nextTick);

        // Insert measurement into the queue
        auto data = acquire();

        if (data.has_value())
        {
            GNSSQueue_.push(*data);
        }

        // Preserve scheduled acquisition ticks instead of skipping delayed periods.
        nextTick += period;
    }

}

std::optional<GNSSData> GNSSSensor::acquire()
{
    GNSSData measurement{};

    const auto now = Clock::now();

    // Simulated hardware reading through I2C
    bool success = bus_.readGNSS(
        i2cAddress_,
        measurement.latitude,
        measurement.longitude,
        measurement.altitude,
        now
    );

    if (!success)
    {
        return std::nullopt;
    }

    measurement.sensorID = sensorID_;
    measurement.measurementID = measurementID_++;
    measurement.timestamp = now;
    measurement.fixValid = true;

    return measurement;
}
