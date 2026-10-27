#pragma once

#include <array>
#include <atomic>
#include <mutex>
#include <optional>

#include "SensorData.hpp"
#include "Settings.hpp"

enum class Health
{
    UNKNOWN,
    HEALTHY,
    FAILED
};

struct FDIRStatus
{
    Health processing = Health::UNKNOWN;

    std::array<Health, Settings::NUM_IMUS> imuSensors{};
    std::array<Health, Settings::NUM_GNSS> gnssSensors{};

    Health imuOutput = Health::UNKNOWN;
    Health gnssOutput = Health::UNKNOWN;
};

class FDIR
{
private:
    std::mutex mutex_;

    // Last processing heartbeat
    std::optional<Clock::time_point> lastHeartbeat_;

    // Current failure status
    FDIRStatus status_;

    Clock::time_point startTime_;

public:
    explicit FDIR(Clock::time_point startTime)
        : startTime_(startTime)
    {
    }

    void reportCycle(
        const std::array<std::optional<Clock::time_point>,
                         Settings::NUM_IMUS>& imuTimes,

        const std::array<std::optional<Clock::time_point>,
                         Settings::NUM_GNSS>& gnssTimes,

        bool imuValid,
        bool gnssValid
    );

    void run(const std::atomic<bool>& running);
};