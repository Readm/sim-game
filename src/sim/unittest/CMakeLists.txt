# 添加测试可执行文件
add_executable(sim_unittest
    test_common.cpp
    test_error.cpp
    type_test.cpp
    packet_test.cpp
    node_test.cpp
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

# 添加测试
add_test(NAME sim_unittest COMMAND sim_unittest) 