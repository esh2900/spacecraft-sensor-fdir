#pragma once
#include "SimulatedI2CBus.hpp"
#include <chrono>

enum class TestScenario {
    Nominal,
    SequentialIMUFailure,
    GNSSDropout,
    TotalGNSSFailure,
    ProcessingFailure
};

inline void configureScenario(SimulatedI2CBus& bus, TestScenario scenario) {
    using DeviceType = SimulatedI2CBus::DeviceType;
    using namespace std::chrono_literals;

    bus.registerSensor(0x68, DeviceType::IMU);
    bus.registerSensor(0x69, DeviceType::IMU);
    bus.registerSensor(0x6A, DeviceType::IMU);
    bus.registerSensor(0x58, DeviceType::GNSS);
    bus.registerSensor(0x59, DeviceType::GNSS);

    switch (scenario) {
    case TestScenario::Nominal:
        break;
    case TestScenario::SequentialIMUFailure:
        bus.addDropout(0x68, 3s);   // IMU 1 permanently offline from t=3s
        bus.addDropout(0x69, 5s);   // IMU 2 permanently offline from t=5s
        bus.addDropout(0x6A, 7s);   // IMU 3 permanently offline from t=7s
        break;
    case TestScenario::GNSSDropout:
        bus.addDropout(0x58, 4s, 4500ms);
        bus.addDropout(0x59, 4s, 4500ms);
        break;
    case TestScenario::TotalGNSSFailure:
        bus.addDropout(0x58, 3s);  // GNSS 1 permanently offline
        bus.addDropout(0x59, 3s);  // GNSS 2 permanently offline
        break;
    case TestScenario::ProcessingFailure:
        // Processing failure is injected directly in main.cpp
        break;
    }
}
