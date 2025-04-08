cmake_minimum_required(VERSION 3.12)
project(FlowSimulator)

# 设置 imgui-node-editor 根目录
set(IMGUI_NODE_EDITOR_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui-node-editor)

# 添加 dl 库
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_library(DL_LIBRARY dl REQUIRED)

# 设置编译选项
option(IMGUI_NODE_EDITOR_STANDALONE "Build standalone examples" ON)
option(IMGUI_NODE_EDITOR_STATIC "Build static library" ON)

# 设置 CMake 模块路径
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# 下载依赖项
include(FetchContent)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)

FetchContent_Declare(
    scope_guard
    GIT_REPOSITORY https://github.com/Neargye/scope_guard.git
    GIT_TAG master
)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)

FetchContent_MakeAvailable(stb scope_guard doctest)

# 添加 imgui-node-editor 库
add_subdirectory(${IMGUI_NODE_EDITOR_ROOT_DIR})

# 创建 stb_image 包配置
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_SOURCE_DIR})

# 创建 ScopeGuard 包配置
add_library(ScopeGuard INTERFACE)
target_include_directories(ScopeGuard INTERFACE ${scope_guard_SOURCE_DIR}/include)

# 添加 application 库
add_subdirectory(${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/application)

# 启用测试
enable_testing()

# 添加 unittest
add_subdirectory(src/sim/unittest)

# 定义函数来添加示例
function(add_example_executable name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE imgui-node-editor application ${DL_LIBRARY})
    target_include_directories(${name} PRIVATE 
        ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example
        ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/application/include
    )
endfunction()

# 添加 blueprints-example
add_example_executable(blueprints-example
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/blueprints-example.cpp
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/utilities/builders.h
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/utilities/drawing.h
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/utilities/widgets.h
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/utilities/builders.cpp
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/utilities/drawing.cpp
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/utilities/widgets.cpp
)

# 复制数据文件
file(COPY ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/data/ DESTINATION ${CMAKE_BINARY_DIR}/data)
file(COPY ${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/blueprints-example/data/ DESTINATION ${CMAKE_BINARY_DIR}/data)

# 链接 dl 库到所有目标
if(DL_LIBRARY)
    link_libraries(${DL_LIBRARY})
endif() 