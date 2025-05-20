# 启用测试
enable_testing()

# 添加测试可执行文件
add_executable(sim_unittest
    test_common.cpp
    test_error.cpp
    type_test.cpp
    packet_test.cpp
    node_test.cpp
    port_test.cpp
    gui_node_test.cpp
)

# 添加服务器测试可执行文件
add_executable(server_test
    server_test.cpp
)

# 添加网络模拟测试可执行文件
add_executable(sim_network_test
    sim_network_test.cpp
)

# 链接doctest库和nlohmann_json库
target_link_libraries(sim_unittest
    PRIVATE doctest::doctest
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE pthread
    PRIVATE sim_lib
)

# 链接服务器测试
target_link_libraries(server_test
    PRIVATE doctest::doctest
    PRIVATE sim_lib
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE httplib::httplib
)

# 链接网络模拟测试
target_link_libraries(sim_network_test
    PRIVATE doctest::doctest
    PRIVATE sim_lib
    PRIVATE nlohmann_json::nlohmann_json
)

# 添加包含目录
target_include_directories(sim_unittest
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${PROJECT_ROOT_DIR}/external/json/include
    PRIVATE ${PROJECT_ROOT_DIR}/external/cpp-httplib
)

target_include_directories(server_test
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${PROJECT_ROOT_DIR}/external/json/include
    PRIVATE ${PROJECT_ROOT_DIR}/external/cpp-httplib
)

target_include_directories(sim_network_test
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${PROJECT_ROOT_DIR}/external/json/include
    PRIVATE ${PROJECT_ROOT_DIR}/external/cpp-httplib
)

# 为每个测试文件添加独立的测试，添加 -s 选项显示成功的测试用例详情
add_test(NAME test_common COMMAND sim_unittest --test-case="测试错误处理系统" -s)
add_test(NAME test_error COMMAND sim_unittest --test-case="Error System" -s)
add_test(NAME test_type COMMAND sim_unittest --test-case="Type System" -s)
add_test(NAME test_packet COMMAND sim_unittest --test-case="Packet System" -s)
add_test(NAME test_node_base COMMAND sim_unittest --test-case="Node Base Class" -s)
add_test(NAME test_node_children COMMAND sim_unittest --test-case="Node Children Management" -s)
add_test(NAME test_node_port COMMAND sim_unittest --test-case="Node Port Management" -s)
add_test(NAME test_node_ticktock COMMAND sim_unittest --test-case="Node TickTock System" -s)
add_test(NAME test_node_serialization COMMAND sim_unittest --test-case="Node Serialization" -s)
add_test(NAME test_node_parallel COMMAND sim_unittest --test-case="Node Parallel Execution" -s)
add_test(NAME test_port_base COMMAND sim_unittest --test-case="Port Base Class" -s)
add_test(NAME test_port_input COMMAND sim_unittest --test-case="Input Port" -s)
add_test(NAME test_port_output COMMAND sim_unittest --test-case="Output Port" -s)
add_test(NAME test_port_serialization COMMAND sim_unittest --test-case="Port Serialization" -s)
add_test(NAME test_node_simulation COMMAND sim_unittest --test-case="Node Simulation" -s)
add_test(NAME test_gui_node COMMAND sim_unittest --test-case="Generate GUI test JSON" -s)

# 添加服务器测试和网络模拟测试
add_test(NAME server_test COMMAND server_test)
add_test(NAME sim_network_test COMMAND sim_network_test)

# 配置测试
if(NOT TARGET doctest::doctest)
    add_subdirectory(${PROJECT_ROOT_DIR}/external/doctest ${CMAKE_BINARY_DIR}/doctest)
endif()

# 配置 cpp-httplib
if(NOT TARGET httplib::httplib)
    add_subdirectory(${PROJECT_ROOT_DIR}/external/cpp-httplib ${CMAKE_BINARY_DIR}/cpp-httplib)
endif() 