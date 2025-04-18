cmake_minimum_required(VERSION 3.10)
project(sim_game)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置OpenGL偏好
set(OpenGL_GL_PREFERENCE GLVND)

# 设置第三方库目录
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

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