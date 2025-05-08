cmake_minimum_required(VERSION 3.10)
project(sim)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加外部依赖目录
set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR}/../..)
set(EXTERNAL_DIR ${PROJECT_ROOT_DIR}/external)

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
    add_subdirectory(${EXTERNAL_DIR}/json ${CMAKE_BINARY_DIR}/json)
endif()

# 配置 cpp-httplib
if(NOT TARGET httplib::httplib)
    add_subdirectory(${EXTERNAL_DIR}/cpp-httplib ${CMAKE_BINARY_DIR}/cpp-httplib)
endif()

# 核心源文件
set(SIM_SOURCES
    src/server.cpp
    src/main.cpp
)

# 编译库
add_library(sim_lib STATIC ${SIM_SOURCES})
target_link_libraries(sim_lib PRIVATE Threads::Threads httplib::httplib)
target_include_directories(sim_lib 
    PRIVATE 
    ${EXTERNAL_DIR}/json/include
    ${EXTERNAL_DIR}/cpp-httplib
)

# 添加可执行文件
add_executable(sim_server ${SIM_SOURCES})
target_link_libraries(sim_server PRIVATE sim_lib Threads::Threads httplib::httplib)
target_include_directories(sim_server 
    PRIVATE 
    ${EXTERNAL_DIR}/json/include
    ${EXTERNAL_DIR}/cpp-httplib
)

# 安装规则
install(TARGETS sim_lib sim_server DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)

# 包含测试目录的CMakeLists
add_subdirectory(unittest) 