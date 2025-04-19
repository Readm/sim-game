# 查找必要的包
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# 设置包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/imgui-node-editor/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui-node-editor
    ${CMAKE_SOURCE_DIR}/external/imgui-node-editor/examples/application/source
    ${CMAKE_SOURCE_DIR}/external/json/include
    ${CMAKE_SOURCE_DIR}/external/doctest
    ${OPENGL_INCLUDE_DIR}
)

# 添加源文件
set(GUI_SOURCES
    src/node_editor.cpp
    src/node.cpp
    src/pin.cpp
    src/link.cpp
    src/node_gui.cpp
)

# 添加测试源文件
set(GUI_TEST_SOURCES
    tests/main.cpp
    tests/test_node_editor.cpp
    tests/test_node.cpp
    tests/test_utils.cpp
    tests/test_node_gui.cpp
    ${CMAKE_SOURCE_DIR}/external/doctest/doctest/doctest.h
)

# 创建GUI库
add_library(sim_gui STATIC ${GUI_SOURCES})

# 链接依赖库
target_link_libraries(sim_gui
    PRIVATE
    imgui
    imgui-node-editor
    glfw
    ${OPENGL_LIBRARIES}
)

# 创建测试可执行文件
add_executable(node_gui_test ${GUI_TEST_SOURCES})
target_include_directories(node_gui_test PRIVATE ${CMAKE_SOURCE_DIR}/external/doctest)
target_link_libraries(node_gui_test
    PRIVATE
    sim_gui
    glfw
    ${OPENGL_LIBRARIES}
)

# 添加测试
add_test(NAME node_gui_test COMMAND node_gui_test) 