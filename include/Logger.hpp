#pragma once

#include <iomanip>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>

#include "FDIR.hpp"
#include "SensorData.hpp"

class ProcessingLogger
{
private:
    std::ofstream file_;

public:
    explicit ProcessingLogger(const std::string& filename)
        : file_(filename)
    {
        if (!file_)
            throw std::runtime_error("Cannot open log file");

        file_ << "time_s,gnss_valid,latitude,longitude,altitude,"
                 "imu_valid,wx,wy,wz,ax,ay,az\n";
    }

    void log(double time,
             const std::optional<GNSSOutput>& gnss,
             const std::optional<IMUOutput>& imu)
    {
        file_ << time << ',' << gnss.has_value();

        if (gnss)
        {
            file_ << ',' << gnss->latitude
                  << ',' << gnss->longitude
                  << ',' << gnss->altitude;
        }
        else
        {
            file_ << ",,,";
        }

        file_ << ',' << imu.has_value();

        if (imu)
        {
            file_ << ',' << imu->angularVelocity.x
                  << ',' << imu->angularVelocity.y
                  << ',' << imu->angularVelocity.z
                  << ',' << imu->acceleration.x
                  << ',' << imu->acceleration.y
                  << ',' << imu->acceleration.z;
        }
        else
        {
            file_ << ",,,,,,";
        }

        file_ << '\n';
    }
};


class FDIRLogger
{
private:
    std::ofstream file_;

    // Convert Health enum to string
    static const char* toString(Health health)
    {
        switch (health)
        {
            case Health::HEALTHY: return "HEALTHY";
            case Health::FAILED:  return "FAILED";
            case Health::UNKNOWN: return "UNKNOWN";
        }

        return "UNKNOWN";
    }

public:
    explicit FDIRLogger(const std::string& filename)
        : file_(filename)
    {
        if (!file_)
            throw std::runtime_error("Cannot open FDIR log file");

        file_ << "time_s,processing,"
                 "imu1,imu2,imu3,"
                 "gnss1,gnss2,"
                 "imu_output,gnss_output\n";

        file_ << std::fixed << std::setprecision(6);
    }

    void log(Clock::time_point now,
             Clock::time_point startTime,
             const FDIRStatus& status)
    {
        double time = std::chrono::duration<double>(
            now - startTime
        ).count();

        file_ << time
              << ',' << toString(status.processing)
              << ',' << toString(status.imuSensors[0])
              << ',' << toString(status.imuSensors[1])
              << ',' << toString(status.imuSensors[2])
              << ',' << toString(status.gnssSensors[0])
              << ',' << toString(status.gnssSensors[1])
              << ',' << toString(status.imuOutput)
              << ',' << toString(status.gnssOutput)
              << '\n';
    }
};