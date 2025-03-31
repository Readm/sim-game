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


# Link libraries
add_executable(sim_game gui/network_viewer.cpp)
target_sources(sim_game PRIVATE ${IMGUI_SOURCES})
target_include_directories(sim_game PRIVATE ${INCLUDE_DIRS})
target_link_libraries(sim_game ${LINK_LIBS})
