# 添加测试可执行文件
add_executable(sim_unittest
    test_common.cpp
    test_error.cpp
)

# 链接doctest库
target_link_libraries(sim_unittest
    PRIVATE doctest
)

# 添加包含目录
target_include_directories(sim_unittest
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${doctest_SOURCE_DIR}/doctest
)

# 添加测试
add_test(NAME sim_unittest COMMAND sim_unittest) 