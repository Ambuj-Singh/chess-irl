cmake_minimum_required(VERSION 3.20)

project(ChessIRL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 CONFIG REQUIRED)

add_executable(
    ChessIRL
    src/main.cpp
    src/core/App.cpp
    src/dashboard/Dashboard.cpp
)

target_include_directories(
    ChessIRL
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(
    ChessIRL
    PRIVATE
    SDL3::SDL3
)