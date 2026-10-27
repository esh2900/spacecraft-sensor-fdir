#include<chrono>
#include<thread>

#include "FDIR.hpp"
#include "DataProcessing.hpp"
#include "Logger.hpp"
#include "SensorData.hpp"
#include "Settings.hpp"

std::optional<GNSSOutput> DataProcessing::processGNSS(){

    // Fetch available data
    auto gnss_data=gnssQueue_.takeAll();

    // Update the latest valid GNSS measurement
    for (const auto& data : gnss_data)
    {
        // Update last sensor timestamp for FDIR
        auto& lastTime = lastGNSSTime_[data.sensorID - 1];

        if (!lastTime || data.timestamp > *lastTime)
        {
            lastTime = data.timestamp;
        }


        if (data.fixValid &&
            (!latestGNSS_ || data.timestamp > latestGNSS_->timestamp))
        {
            latestGNSS_ = data;
        }
    }

    // Check if the latest measurement is still valid
    // Reuse the latest valid GNSS measurement while it remains within the configured age tolerance.
    if (latestGNSS_ && Clock::now() - latestGNSS_->timestamp <= std::chrono::seconds(Settings::GNSS_TOLERANCE))
    {
        // Valid GNSS data
        return GNSSOutput{
            latestGNSS_->latitude,
            latestGNSS_->longitude,
            latestGNSS_->altitude,
            latestGNSS_->timestamp
        };
    }
    else
    {
        // Outdated GNSS data
        return std::nullopt;
    }   

}

std::optional<IMUOutput> DataProcessing::processIMU()
{
    auto imu_data = imuQueue_.takeAll();
    
    const auto now = Clock::now();
    const auto maxAge = std::chrono::milliseconds(Settings::IMU_MAX_AGE_MS);

    xyzvector accelerationSum{};
    xyzvector angularVelocitySum{};
    int count = 0;

    for (const auto& data : imu_data)
    {
        // Update sensor activity timestamp
        auto& lastTime = lastIMUTime_[data.sensorID - 1];

        if (!lastTime || data.timestamp > *lastTime)
            lastTime = data.timestamp;

        // Ignore invalid or outdated measurements
        if (!data.measurementValid || now - data.timestamp > maxAge)
        {
            continue;
        }

        // Sum accelerations
        accelerationSum.x += data.acceleration.x;
        accelerationSum.y += data.acceleration.y;
        accelerationSum.z += data.acceleration.z;

        // Sum angular velocities
        angularVelocitySum.x += data.angularVelocity.x;
        angularVelocitySum.y += data.angularVelocity.y;
        angularVelocitySum.z += data.angularVelocity.z;

        count++;
    }

    // No valid measurements available
    if (count == 0)
    {
        return std::nullopt;
    }

    // Calculate averages
    IMUOutput output;

    output.acceleration = {
        accelerationSum.x / count,
        accelerationSum.y / count,
        accelerationSum.z / count
    };

    output.angularVelocity = {
        angularVelocitySum.x / count,
        angularVelocitySum.y / count,
        angularVelocitySum.z / count
    };

    output.timestamp = now;

    return output;
}

void DataProcessing::run(const std::atomic<bool>& running, Clock::time_point startTime){

    ProcessingLogger logger("processing_log.csv");
    
    auto nextTick = startTime;
    const auto period = std::chrono::nanoseconds(1'000'000'000LL / processingRateHz_);

    while (running.load())
    {
        
        std::this_thread::sleep_until(nextTick);

        if (!running.load())
        break;

        // Process sensor data
        auto gnssOutput = processGNSS();
        auto imuOutput = processIMU();

         // Report processing cycle to FDIR
        fdir_.reportCycle(
            lastIMUTime_,
            lastGNSSTime_,
            imuOutput.has_value(),
            gnssOutput.has_value()
        );

        // Log processed data
        double time = std::chrono::duration<double>(
            Clock::now() - startTime
        ).count();

        logger.log(time, gnssOutput, imuOutput);

        // Advance to the next processing tick
        nextTick += period;
            
    }

}
