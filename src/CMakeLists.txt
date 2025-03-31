cmake_minimum_required(VERSION 3.10)

# Set the project name
project(sim_game)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set CMP0072 policy to avoid OpenGL warnings
if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

# Include dependencies
include(${CMAKE_SOURCE_DIR}/dependencies.cmake)

# Find and link nlohmann_json library
find_package(nlohmann_json REQUIRED)

# Link libraries
add_executable(sim_game gui/network_viewer.cpp)
target_sources(sim_game PRIVATE ${IMGUI_SOURCES})
target_include_directories(sim_game PRIVATE ${INCLUDE_DIRS} ${imgui_SOURCE_DIR})
target_link_libraries(sim_game PRIVATE ${LINK_LIBS} nlohmann_json::nlohmann_json)
