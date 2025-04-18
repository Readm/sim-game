cmake_minimum_required(VERSION 3.10)
project(sim_game)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置OpenGL偏好
set(OpenGL_GL_PREFERENCE GLVND)

# 设置第三方库目录
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

# 添加imgui库
add_library(imgui STATIC
    ${EXTERNAL_DIR}/imgui-node-editor/external/imgui/imgui.cpp
    ${EXTERNAL_DIR}/imgui-node-editor/external/imgui/imgui_draw.cpp
    ${EXTERNAL_DIR}/imgui-node-editor/external/imgui/imgui_tables.cpp
    ${EXTERNAL_DIR}/imgui-node-editor/external/imgui/imgui_widgets.cpp
    ${EXTERNAL_DIR}/imgui-node-editor/external/imgui/imgui_demo.cpp
    ${EXTERNAL_DIR}/imgui-node-editor/examples/application/source/imgui_impl_glfw.cpp
    ${EXTERNAL_DIR}/imgui-node-editor/examples/application/source/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${EXTERNAL_DIR}/imgui-node-editor/external/imgui
    ${EXTERNAL_DIR}/imgui-node-editor/examples/application/source
)

# 添加imgui-node-editor
# 构建imgui-node-editor作为一个库
add_library(imgui-node-editor STATIC
    ${EXTERNAL_DIR}/imgui-node-editor/imgui_node_editor.cpp
    ${EXTERNAL_DIR}/imgui-node-editor/imgui_node_editor_api.cpp
    ${EXTERNAL_DIR}/imgui-node-editor/imgui_canvas.cpp
    ${EXTERNAL_DIR}/imgui-node-editor/crude_json.cpp
)
target_include_directories(imgui-node-editor PUBLIC
    ${EXTERNAL_DIR}/imgui-node-editor
    ${EXTERNAL_DIR}/imgui-node-editor/external/imgui
)
target_link_libraries(imgui-node-editor PUBLIC imgui)

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