#pragma once

#include <atomic>
#include <cstdint>
#include <stdexcept>
#include <optional>

#include "SensorData.hpp"
#include "ThreadSafeQueue.hpp"

class SimulatedI2CBus; // Forward declaration (only for testing)

class IMUSensor
{
    private:
        std::uint8_t sensorID_;
        std::uint64_t measurementID_;
        std::uint8_t i2cAddress_;
        ThreadSafeQueue<IMUData>& IMUQueue_;
        int acquisitionRateHz_;
        SimulatedI2CBus& bus_;  //TestingBus
    
    public:
        IMUSensor(std::uint8_t id,std::uint8_t i2cAddress, 
                  ThreadSafeQueue<IMUData>& imuQueue,int acquisitionRateHz,
                  SimulatedI2CBus& bus) //TestingBus
            : sensorID_(id), measurementID_(0), i2cAddress_(i2cAddress), 
              IMUQueue_(imuQueue), 
              acquisitionRateHz_(acquisitionRateHz), bus_(bus)
        {
            if (acquisitionRateHz_ <= 0 || acquisitionRateHz_ > 1000)
            {
                throw std::invalid_argument("Acquisition rate must be between 1 and 1000 Hz");
            }
        } 
        
        void run(const std::atomic<bool>& running, Clock::time_point startTime);
        std::optional<IMUData> acquire();

};