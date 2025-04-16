cmake_minimum_required(VERSION 3.10)
project(sim_game)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置OpenGL偏好
set(OpenGL_GL_PREFERENCE GLVND)

# 设置第三方库目录
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

# 获取doctest
if(NOT EXISTS ${EXTERNAL_DIR}/doctest)
    message(STATUS "Downloading doctest...")
    execute_process(
        COMMAND git clone --depth 1 --branch v2.4.11 https://github.com/doctest/doctest.git ${EXTERNAL_DIR}/doctest
        RESULT_VARIABLE GIT_RESULT
    )
    if(NOT GIT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone doctest")
    endif()
endif()

# 获取cpp-httplib
if(NOT EXISTS ${EXTERNAL_DIR}/cpp-httplib)
    message(STATUS "Downloading cpp-httplib...")
    execute_process(
        COMMAND git clone --depth 1 --branch v0.12.3 https://github.com/yhirose/cpp-httplib.git ${EXTERNAL_DIR}/cpp-httplib
        RESULT_VARIABLE GIT_RESULT
    )
    if(NOT GIT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone cpp-httplib")
    endif()
endif()

# 获取nlohmann/json
if(NOT EXISTS ${EXTERNAL_DIR}/json)
    message(STATUS "Downloading nlohmann/json...")
    execute_process(
        COMMAND git clone --depth 1 --branch v3.11.2 https://github.com/nlohmann/json.git ${EXTERNAL_DIR}/json
        RESULT_VARIABLE GIT_RESULT
    )
    if(NOT GIT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone nlohmann/json")
    endif()
endif()

# 获取imgui-node-editor
if(NOT EXISTS ${EXTERNAL_DIR}/imgui-node-editor)
    message(STATUS "Downloading imgui-node-editor...")
    execute_process(
        COMMAND git clone --depth 1 https://github.com/thedmd/imgui-node-editor.git ${EXTERNAL_DIR}/imgui-node-editor
        RESULT_VARIABLE GIT_RESULT
    )
    if(NOT GIT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone imgui-node-editor")
    endif()
endif()

# 添加imgui-node-editor
add_subdirectory(${EXTERNAL_DIR}/imgui-node-editor)

# 设置包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${EXTERNAL_DIR}
    ${EXTERNAL_DIR}/doctest
    ${EXTERNAL_DIR}/cpp-httplib
    ${EXTERNAL_DIR}/json/include
)

# 添加子项目
add_subdirectory(src/sim)
add_subdirectory(src/gui)

# 设置编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 启用测试
enable_testing() 