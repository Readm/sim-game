#include "doctest/doctest.h"
#include "common.h"
#include <sstream>
#include <string>

TEST_CASE("测试错误处理系统") {
    using namespace sim;

    SUBCASE("测试错误类型枚举") {
        // 验证错误类型的存在性
        ErrorType debug = ErrorType::DEBUG;
        ErrorType info = ErrorType::INFO;
        ErrorType warning = ErrorType::WARNING;
        ErrorType error = ErrorType::ERROR;
        ErrorType fatal = ErrorType::FATAL;
        
        // 验证错误类型的顺序
        CHECK(static_cast<int>(ErrorType::DEBUG) == 0);
        CHECK(static_cast<int>(ErrorType::INFO) == 1);
        CHECK(static_cast<int>(ErrorType::WARNING) == 2);
        CHECK(static_cast<int>(ErrorType::ERROR) == 3);
        CHECK(static_cast<int>(ErrorType::FATAL) == 4);
    }
    
    SUBCASE("测试非异常错误报告") {
        // 测试不会抛出异常的错误类型
        MESSAGE("测试非异常错误类型：");
        SIM_DEBUG("这是一条调试信息");
        SIM_INFO("这是一条普通信息");
        SIM_WARNING("这是一条警告信息");
        CHECK(true); // 如果能执行到这里，说明错误报告功能正常
    }

    SUBCASE("测试异常错误报告") {
        // 测试会抛出异常的错误类型
        CHECK_THROWS_AS(SIM_ERROR("这是一条错误信息"), std::runtime_error);
        CHECK_THROWS_AS(SIM_FATAL("这是一条致命错误信息"), std::runtime_error);
    }
} 