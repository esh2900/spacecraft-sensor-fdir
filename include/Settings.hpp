#pragma once

#include <cstddef>
#include <chrono>

namespace Settings
{
    inline constexpr std::size_t NUM_IMUS = 3;
    inline constexpr std::size_t NUM_GNSS = 2;

    inline constexpr int IMU_RATE_HZ = 100;
    inline constexpr int GNSS_RATE_HZ = 20;
    inline constexpr int PROCESSING_RATE_HZ = 50;

    inline constexpr int MAX_MISSED_INTERVALS = 3;
    inline constexpr int PROCESSING_WATCHDOG_INTERVALS = 3;
    inline constexpr int GNSS_TOLERANCE = 1; // seconds
    inline constexpr int IMU_MAX_AGE_MS = 30;//miliseconds

    // FDIR Tolerances
    inline constexpr auto IMU_TIMEOUT =
    std::chrono::milliseconds(
        1000 * MAX_MISSED_INTERVALS / IMU_RATE_HZ
    );

    inline constexpr auto GNSS_TIMEOUT =
        std::chrono::milliseconds(
            1000 * MAX_MISSED_INTERVALS / GNSS_RATE_HZ
        );

    inline constexpr auto PROCESSING_WATCHDOG_TIMEOUT =
        std::chrono::milliseconds(
            1000 * PROCESSING_WATCHDOG_INTERVALS / PROCESSING_RATE_HZ
        );
}