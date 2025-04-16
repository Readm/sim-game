cmake_minimum_required(VERSION 3.10)
project(sim)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置JSON库
include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

# 包含目录
include_directories(include)

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

# 查找依赖
find_package(Threads REQUIRED)

# 编译库
add_library(${PROJECT_NAME} STATIC ${SIM_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads nlohmann_json::nlohmann_json)
target_include_directories(${PROJECT_NAME} PRIVATE ${nlohmann_json_SOURCE_DIR}/include)

# 编译服务器测试
add_executable(server_test ${SERVER_TEST_SOURCES})
target_link_libraries(server_test PRIVATE ${PROJECT_NAME})
target_include_directories(server_test PRIVATE ${nlohmann_json_SOURCE_DIR}/include)

# 编译网络模拟测试
add_executable(sim_network_test ${SIM_NETWORK_TEST_SOURCES})
target_link_libraries(sim_network_test PRIVATE ${PROJECT_NAME})
target_include_directories(sim_network_test PRIVATE ${nlohmann_json_SOURCE_DIR}/include)

# 添加测试
enable_testing()
add_test(NAME server_test COMMAND server_test)
add_test(NAME sim_network_test COMMAND sim_network_test)

# 安装规则
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(DIRECTORY include/ DESTINATION include) 