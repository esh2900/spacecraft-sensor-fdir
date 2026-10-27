#include <iostream>
#include <cassert>
#include <chrono>
#include <stdexcept>

#include "simulation/SimulatedI2CBus.hpp"

using namespace std::chrono_literals;

// Test 1: Valid sensor readings
void testValidReadings()
{
    auto start = Clock::now();
    SimulatedI2CBus bus(start);

    bus.registerSensor(0x68, SimulatedI2CBus::DeviceType::IMU);
    bus.registerSensor(0x58, SimulatedI2CBus::DeviceType::GNSS);

    xyzvector acceleration, angularVelocity;
    double latitude, longitude, altitude;

    bool imuValid = bus.readIMU(
        0x68, acceleration, angularVelocity, start + 10ms
    );

    bool gnssValid = bus.readGNSS(
        0x58, latitude, longitude, altitude, start + 50ms
    );

    assert(imuValid);
    assert(gnssValid);
}

// Test 2: Deterministic measurements
void testDeterministicMeasurements()
{
    auto start = Clock::now();
    SimulatedI2CBus bus(start);

    bus.registerSensor(0x68, SimulatedI2CBus::DeviceType::IMU);

    xyzvector acc1, gyro1;
    xyzvector acc2, gyro2;

    auto measurementTime = start + 100ms;

    assert(bus.readIMU(0x68, acc1, gyro1, measurementTime));
    assert(bus.readIMU(0x68, acc2, gyro2, measurementTime));

    assert(acc1.x == acc2.x);
    assert(acc1.y == acc2.y);
    assert(acc1.z == acc2.z);

    assert(gyro1.x == gyro2.x);
    assert(gyro1.y == gyro2.y);
    assert(gyro1.z == gyro2.z);
}

// Test 3: Temporary dropout
void testTemporaryDropout()
{
    auto start = Clock::now();
    SimulatedI2CBus bus(start);

    bus.registerSensor(0x68, SimulatedI2CBus::DeviceType::IMU);

    // Sensor unavailable between 100 ms and 200 ms
    bus.addDropout(0x68, 100ms, 200ms);

    xyzvector acceleration, angularVelocity;

    assert(bus.readIMU(
        0x68, acceleration, angularVelocity, start + 50ms
    ));

    assert(!bus.readIMU(
        0x68, acceleration, angularVelocity, start + 150ms
    ));

    assert(bus.readIMU(
        0x68, acceleration, angularVelocity, start + 250ms
    ));
}

// Test 4: Permanent dropout
void testPermanentDropout()
{
    auto start = Clock::now();
    SimulatedI2CBus bus(start);

    bus.registerSensor(0x58, SimulatedI2CBus::DeviceType::GNSS);

    // Permanent failure starting at 100 ms
    bus.addDropout(0x58, 100ms);

    double latitude, longitude, altitude;

    assert(bus.readGNSS(
        0x58, latitude, longitude, altitude, start + 50ms
    ));

    assert(!bus.readGNSS(
        0x58, latitude, longitude, altitude, start + 150ms
    ));

    assert(!bus.readGNSS(
        0x58, latitude, longitude, altitude, start + 1000ms
    ));
}

// Test 5: Duplicate sensor address
void testDuplicateAddress()
{
    auto start = Clock::now();
    SimulatedI2CBus bus(start);

    bus.registerSensor(0x68, SimulatedI2CBus::DeviceType::IMU);

    bool exceptionThrown = false;

    try
    {
        bus.registerSensor(0x68, SimulatedI2CBus::DeviceType::GNSS);
    }
    catch (const std::invalid_argument&)
    {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}

int main()
{
    testValidReadings();
    testDeterministicMeasurements();
    testTemporaryDropout();
    testPermanentDropout();
    testDuplicateAddress();

    std::cout << "All SimulatedI2CBus tests passed!\n";

    return 0;
}
