#include <thread>

#include "Logger.hpp"
#include "FDIR.hpp"

void FDIR::reportCycle(
    const std::array<std::optional<Clock::time_point>,
                     Settings::NUM_IMUS>& imuTimes,
    const std::array<std::optional<Clock::time_point>,
                     Settings::NUM_GNSS>& gnssTimes,
    bool imuValid,
    bool gnssValid)
{
    const auto now = Clock::now();

    std::lock_guard<std::mutex> lock(mutex_);

    // Processing heartbeat
    lastHeartbeat_ = now;
    status_.processing = Health::HEALTHY;

    // Check IMU
    for (std::size_t i = 0; i < Settings::NUM_IMUS; i++)
    {
        if (!imuTimes[i])
        {
            status_.imuSensors[i] =
                (now - startTime_ >= Settings::IMU_TIMEOUT)
                ? Health::FAILED : Health::UNKNOWN;
        }
        else
        {
            status_.imuSensors[i] =
                (now - *imuTimes[i] >= Settings::IMU_TIMEOUT)
                ? Health::FAILED : Health::HEALTHY;        
        }
    }

    // Check GNSS
    for (std::size_t i = 0; i < Settings::NUM_GNSS; i++)
    {
        if (!gnssTimes[i])
        {
            status_.gnssSensors[i] =
                (now - startTime_ >= Settings::GNSS_TIMEOUT)
                ? Health::FAILED : Health::UNKNOWN;
        }
        else
        {
            status_.gnssSensors[i] =
                (now - *gnssTimes[i] >= Settings::GNSS_TIMEOUT)
                ? Health::FAILED : Health::HEALTHY;
        }
    }

    // Check processed outputs
    status_.imuOutput = imuValid
        ? Health::HEALTHY : Health::FAILED;

    status_.gnssOutput = gnssValid
        ? Health::HEALTHY : Health::FAILED;
}


void FDIR::run(const std::atomic<bool>& running)
{
    auto nextTick = startTime_;

    const auto period = std::chrono::milliseconds(20);
    const auto timeout = std::chrono::milliseconds(Settings::PROCESSING_WATCHDOG_TIMEOUT);

    FDIRLogger logger("fdir_log.csv");

    while (running.load())
    {
        std::this_thread::sleep_until(nextTick);

        if (!running.load())
            break;

        const auto now = Clock::now();

        FDIRStatus snapshot;

        {
            std::lock_guard<std::mutex> lock(mutex_);

            // Processing watchdog
            if ((!lastHeartbeat_ &&
                 now - startTime_ >= timeout) ||
                (lastHeartbeat_ &&
                 now - *lastHeartbeat_ >= timeout))
            {
                status_.processing = Health::FAILED;

                // Sensor states are no longer reliable
                status_.imuSensors.fill(Health::UNKNOWN);
                status_.gnssSensors.fill(Health::UNKNOWN);

                status_.imuOutput = Health::UNKNOWN;
                status_.gnssOutput = Health::UNKNOWN;
            }

            snapshot = status_;
        }

        // Log FDIR status
        logger.log(now, startTime_, snapshot);

        nextTick += period;
    }
}
