#pragma once

#include <array>
#include <atomic>
#include <optional>
#include <stdexcept>

#include "FDIR.hpp"
#include "Settings.hpp"
#include "SensorData.hpp"
#include "ThreadSafeQueue.hpp"

class DataProcessing
{
private:
    
    int processingRateHz_;

    // Shared queues
    ThreadSafeQueue<IMUData>& imuQueue_;
    ThreadSafeQueue<GNSSData>& gnssQueue_;
    
    // GNSS
    std::optional<GNSSData> latestGNSS_; // Saves Last Valid GNSS value

    // Sensor Activity Monitoring
    // tracking last valid  timestamp 
    std::array<std::optional<Clock::time_point>,Settings::NUM_IMUS> lastIMUTime_; // for each IMU.
    std::array<std::optional<Clock::time_point>,Settings::NUM_GNSS> lastGNSSTime_; // for each GNSS.
    // Fault Detection Report Function
    FDIR& fdir_;

    // Internal processing
    std::optional<IMUOutput> processIMU();
    std::optional<GNSSOutput> processGNSS();

public:
    DataProcessing(ThreadSafeQueue<IMUData>& imuQueue, ThreadSafeQueue<GNSSData>& gnssQueue,
                   int processingRateHz, FDIR& fdir)
        : processingRateHz_(processingRateHz),
          imuQueue_(imuQueue), gnssQueue_(gnssQueue),
          fdir_(fdir)
    {
        if (processingRateHz_ <= 0 || processingRateHz_ > 1000)
        {
            throw std::invalid_argument(
                "Processing rate must be between 1 and 1000 Hz"
            );
        }
    }

    void run(const std::atomic<bool>& running, 
             Clock::time_point startTime);

};