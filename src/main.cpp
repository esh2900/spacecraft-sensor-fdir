#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>

#include "Settings.hpp"
#include "SensorData.hpp"
#include "ThreadSafeQueue.hpp"
#include "IMUSensor.hpp"
#include "GNSSSensor.hpp"
#include "DataProcessing.hpp"
#include "FDIR.hpp"

//Simulation Includes
#include <filesystem>
#include <string>
#include "SimulatedI2CBus.hpp"
#include "ScenarioConfiguration.hpp"

void runScenario(TestScenario scenario)
{
    using namespace std::chrono_literals;

    // Shared queues
    ThreadSafeQueue<IMUData> imuQueue;
    ThreadSafeQueue<GNSSData> gnssQueue;

    // Simulation control
    std::atomic<bool> running{true};
    std::atomic<bool> processingRunning{true};

    const auto startTime = Clock::now() + 100ms;
    const auto endTime = startTime + 10s;

    // Simulated hardware
    SimulatedI2CBus bus(startTime);
    configureScenario(bus, scenario);

    // IMU sensors
    IMUSensor imu1(1, 0x68, imuQueue, Settings::IMU_RATE_HZ, bus);
    IMUSensor imu2(2, 0x69, imuQueue, Settings::IMU_RATE_HZ, bus);
    IMUSensor imu3(3, 0x6A, imuQueue, Settings::IMU_RATE_HZ, bus);

    // GNSS sensors
    GNSSSensor gnss1(1, 0x58, gnssQueue, Settings::GNSS_RATE_HZ, bus);
    GNSSSensor gnss2(2, 0x59, gnssQueue, Settings::GNSS_RATE_HZ, bus);

    // FDIR
    FDIR fdir(startTime);

    // DataProcessing
    DataProcessing processing(
        imuQueue,
        gnssQueue,
        Settings::PROCESSING_RATE_HZ,
        fdir
    );

    // Start threads
    std::thread imuThread1(&IMUSensor::run, &imu1, std::cref(running), startTime);
    std::thread imuThread2(&IMUSensor::run, &imu2, std::cref(running), startTime);
    std::thread imuThread3(&IMUSensor::run, &imu3, std::cref(running), startTime);

    std::thread gnssThread1(&GNSSSensor::run, &gnss1, std::cref(running), startTime);
    std::thread gnssThread2(&GNSSSensor::run, &gnss2, std::cref(running), startTime);

    std::thread processingThread(
        &DataProcessing::run, &processing,
        std::cref(processingRunning), startTime
    );

    std::thread fdirThread(
        &FDIR::run, &fdir, std::cref(running)
    );

    std::cout << "Simulation started...\n";

    // Inject processing failure at t = 5s for ProcessingFailure case
    if (scenario == TestScenario::ProcessingFailure)
    {
        std::this_thread::sleep_until(startTime + 5s);

        std::cout << "Injecting processing failure at t = 5s\n";

        processingRunning.store(false);
        processingThread.join();
    }

    // Wait until simulation ends
    std::this_thread::sleep_until(endTime);

    // Stop all remaining threads
    running.store(false);
    processingRunning.store(false);

    // Wait for completion
    imuThread1.join();
    imuThread2.join();
    imuThread3.join();

    gnssThread1.join();
    gnssThread2.join();

    if (processingThread.joinable())
        processingThread.join();

    fdirThread.join();

    std::cout << "Simulation finished.\n";
}

void renameLogs(const std::string& scenarioName)
{
    namespace fs = std::filesystem;

    fs::create_directories("results");

    fs::rename(
        "processing_log.csv",
        "results/" + scenarioName + "_processing_log.csv"
    );

    fs::rename(
        "fdir_log.csv",
        "results/" + scenarioName + "_fdir_log.csv"
    );
}

int main()
{
    std::cout << "===== NOMINAL =====\n";
    runScenario(TestScenario::Nominal);
    renameLogs("nominal");

    std::cout << "===== IMU FAILURE =====\n";
    runScenario(TestScenario::SequentialIMUFailure);
    renameLogs("imu_failure");

    std::cout << "===== GNSS DROPOUT =====\n";
    runScenario(TestScenario::GNSSDropout);
    renameLogs("gnss_dropout");

    std::cout << "===== TOTAL GNSS FAILURE =====\n";
    runScenario(TestScenario::TotalGNSSFailure);
    renameLogs("total_gnss_failure");

    std::cout << "===== PROCESSING FAILURE =====\n";
    runScenario(TestScenario::ProcessingFailure);
    renameLogs("processing_failure");

    std::cout << "===== ALL TESTS COMPLETED =====\n";

    return 0;
}
