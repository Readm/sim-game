cmake_minimum_required(VERSION 3.10)
project(sim)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义项目根目录
if(CMAKE_SOURCE_DIR MATCHES "src/sim$")
    set(PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}/../..")
else()
    set(PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}")
endif()

# 包含目录
include_directories(include)

# 查找依赖
find_package(Threads REQUIRED)

# 配置 doctest
if(NOT TARGET doctest::doctest)
    add_subdirectory(${PROJECT_ROOT_DIR}/external/doctest ${CMAKE_BINARY_DIR}/doctest)
endif()

# 配置 nlohmann_json
if(NOT TARGET nlohmann_json::nlohmann_json)
    add_subdirectory(${PROJECT_ROOT_DIR}/external/json ${CMAKE_BINARY_DIR}/json)
endif()

# 核心源文件
set(SIM_SOURCES
    src/server.cpp
)

# 服务器测试源文件
set(SERVER_TEST_SOURCES
    unittest/server_test.cpp
)

# 网络模拟测试源文件
set(SIM_NETWORK_TEST_SOURCES
    unittest/sim_network_test.cpp
)

# 编译库
add_library(${PROJECT_NAME} STATIC ${SIM_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_ROOT_DIR}/external/json/include)

# 编译服务器测试
add_executable(server_test ${SERVER_TEST_SOURCES})
target_link_libraries(server_test PRIVATE ${PROJECT_NAME} doctest::doctest)
target_include_directories(server_test PRIVATE ${PROJECT_ROOT_DIR}/external/json/include)

# 编译网络模拟测试
add_executable(sim_network_test ${SIM_NETWORK_TEST_SOURCES})
target_link_libraries(sim_network_test PRIVATE ${PROJECT_NAME} doctest::doctest)
target_include_directories(sim_network_test PRIVATE ${PROJECT_ROOT_DIR}/external/json/include)

# 添加测试
enable_testing()
add_test(NAME server_test COMMAND server_test)
add_test(NAME sim_network_test COMMAND sim_network_test)

# 安装规则
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

# 包含测试目录的CMakeLists
add_subdirectory(unittest) 