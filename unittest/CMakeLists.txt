cmake_minimum_required(VERSION 3.14)
project(GUITestProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use dependencies from src/dependencies.cmake
include(${CMAKE_SOURCE_DIR}/../src/dependencies.cmake)

# Combine all test files into a single executable
add_executable(all_tests 
    ${CMAKE_CURRENT_SOURCE_DIR}/test_fbs.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/test_imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_networkviewer.cpp
)

# Link libraries and include directories to all_tests
target_sources(all_tests PRIVATE ${IMGUI_SOURCES})
target_include_directories(all_tests PRIVATE ${INCLUDE_DIRS})
target_link_libraries(all_tests PRIVATE ${LINK_LIBS} OpenGL::GL)

# Add a custom target to run all tests
add_custom_target(test
    COMMAND all_tests
    DEPENDS all_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all tests..."
)
