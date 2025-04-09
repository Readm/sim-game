# 添加测试可执行文件
add_executable(sim_unittest
    test_common.cpp
    test_error.cpp
    type_test.cpp
    packet_test.cpp
    node_test.cpp
    port_test.cpp
)

# 链接doctest库和nlohmann_json库
target_link_libraries(sim_unittest
    PRIVATE doctest::doctest
    PRIVATE nlohmann_json::nlohmann_json
)

# 添加包含目录
target_include_directories(sim_unittest
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# 为每个测试文件添加独立的测试
add_test(NAME test_common COMMAND sim_unittest --test-case="测试错误处理系统")
add_test(NAME test_error COMMAND sim_unittest --test-case="Error System")
add_test(NAME test_type COMMAND sim_unittest --test-case="Type System")
add_test(NAME test_packet COMMAND sim_unittest --test-case="Packet System")
add_test(NAME test_node_base COMMAND sim_unittest --test-case="Node Base Class")
add_test(NAME test_node_children COMMAND sim_unittest --test-case="Node Children Management")
add_test(NAME test_node_port COMMAND sim_unittest --test-case="Node Port Management")
add_test(NAME test_node_ticktock COMMAND sim_unittest --test-case="Node TickTock System")
add_test(NAME test_node_serialization COMMAND sim_unittest --test-case="Node Serialization")
add_test(NAME test_port_base COMMAND sim_unittest --test-case="Port Base Class")
add_test(NAME test_port_input COMMAND sim_unittest --test-case="Input Port")
add_test(NAME test_port_output COMMAND sim_unittest --test-case="Output Port")
add_test(NAME test_port_serialization COMMAND sim_unittest --test-case="Port Serialization") 