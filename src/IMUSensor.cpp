#include <chrono>
#include <atomic>
#include <thread>
#include "IMUSensor.hpp"
#include "SimulatedI2CBus.hpp"

using Clock = std::chrono::steady_clock;

void IMUSensor::run(const std::atomic<bool>& running, Clock::time_point startTime)
{
    
    const auto period = std::chrono::nanoseconds(1'000'000'000LL / acquisitionRateHz_);
    auto nextTick = startTime;

    while (running.load())
    {    
        // Sleep until next scheduled acquisition
        std::this_thread::sleep_until(nextTick);
        
        auto measurement = acquire();

        if (measurement.has_value())
        {
            IMUQueue_.push(*measurement);
        }

        // Preserve scheduled acquisition ticks instead of skipping delayed periods.
        nextTick += period;

    }

}

std::optional<IMUData> IMUSensor::acquire()
{
    IMUData measurement{};
    const auto now = Clock::now();

    // Simulated hardware reading through I2C
    bool success = bus_.readIMU(
        i2cAddress_,
        measurement.acceleration,
        measurement.angularVelocity,
        now
    );

    if (!success)
    {
        return std::nullopt;
    }

    measurement.sensorID = sensorID_;
    measurement.measurementID = measurementID_++;
    measurement.timestamp = now;
    measurement.measurementValid = true;

    return measurement;
}
