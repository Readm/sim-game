cmake_minimum_required(VERSION 3.10)
project(sim_network_viewer)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找必要的包
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Threads REQUIRED)

# 设置doctest
set(DOCTEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/doctest/doctest)
add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE ${DOCTEST_INCLUDE_DIR})
add_library(doctest::doctest ALIAS doctest)

# 源文件
set(SOURCES
    main.cpp
    network_viewer.cpp
    http_client.cpp
)

# 头文件
set(HEADERS
    network_viewer.h
    http_client.h
)

# 测试源文件
set(TEST_SOURCES
    test/gui_test_framework.cpp
    test/network_viewer_test.cpp
)

# 测试头文件
set(TEST_HEADERS
    test/gui_test_framework.h
)

# 创建主可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 创建测试可执行文件
add_executable(${PROJECT_NAME}_test ${TEST_SOURCES} ${TEST_HEADERS} network_viewer.cpp http_client.cpp)

# 链接依赖库
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    imgui-node-editor
    OpenGL::GL
    glfw
    Threads::Threads
)

# 条件性地链接sim库
find_library(SIM_LIBRARY sim)
if(SIM_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PRIVATE sim)
else()
    message(WARNING "sim library not found, some features might be unavailable")
endif()

# 链接测试依赖库
target_link_libraries(${PROJECT_NAME}_test
    PRIVATE
    imgui-node-editor
    OpenGL::GL
    glfw
    Threads::Threads
    doctest::doctest
)

# 条件性地链接sim库
if(SIM_LIBRARY)
    target_link_libraries(${PROJECT_NAME}_test PRIVATE sim)
endif()

# 包含目录
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/imgui-node-editor
    ${CMAKE_SOURCE_DIR}/external/cpp-httplib
)

target_include_directories(${PROJECT_NAME}_test
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/imgui-node-editor
    ${CMAKE_SOURCE_DIR}/external/cpp-httplib
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 启用测试
enable_testing()
add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)

# 添加前后端集成测试
add_executable(network_integration_test
    test/network_integration_test.cpp
    test/mock_server.cpp
    network_client.cpp
    network_viewer.cpp
)

target_link_libraries(network_integration_test
    PRIVATE
    imgui
    imgui-node-editor
    pthread
)

# 添加WebSocket测试
add_executable(websocket_test
    test/websocket_test.cpp
    test/mock_server.cpp
    network_client.cpp
    network_viewer.cpp
)

target_link_libraries(websocket_test
    PRIVATE
    imgui
    imgui-node-editor
    pthread
) 