cmake_minimum_required(VERSION 3.16)

project(FlightSoftwareSimulation LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(flight_sim
    src/main.cpp
    src/IMUSensor.cpp
    src/GNSSSensor.cpp
    src/DataProcessing.cpp
    src/FDIR.cpp
)

# Project Header Files
target_include_directories(flight_sim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/simulation
)

# Find and Link thread support to the flight_sim executable.
# PRIVATE means the dependency is not propagated to other targets.
find_package(Threads REQUIRED)
target_link_libraries(flight_sim PRIVATE Threads::Threads)
