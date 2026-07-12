cmake_minimum_required(VERSION 3.20)

project(ChessIRL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

add_executable(
    ChessIRL
    src/main.cpp
    src/core/App.cpp
    src/dashboard/Dashboard.cpp
    src/chess-game/ChessGame.cpp
    src/chess-game/renderer/ChessRenderer.cpp
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
    SDL3_image::SDL3_image
)

target_compile_definitions(
    ChessIRL
    PRIVATE
    CHESSIRL_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

include(CTest)
if(BUILD_TESTING)
    add_executable(ChessGameTests
        tests/ChessGameTests.cpp
        src/chess-game/ChessGame.cpp
    )
    target_include_directories(ChessGameTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    add_test(NAME chess_rules COMMAND ChessGameTests)
endif()
