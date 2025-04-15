#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "common.h"

TEST_CASE("测试错误报告系统") {
    using namespace sim;

    SUBCASE("基本错误报告功能 - 非异常") {
        // 测试不会抛出异常的错误报告
        MESSAGE("测试非异常错误报告：");
        SIM_DEBUG("这是一条调试信息");
        SIM_INFO("这是一条普通信息");
        SIM_WARNING("这是一条警告信息");
        CHECK(true); // 如果能执行到这里，说明错误报告功能正常
    }

    SUBCASE("错误报告异常测试 - ERROR") {
        // 测试 ERROR 级别异常
        CHECK_THROWS_AS(SIM_ERROR("这是一条错误信息"), std::runtime_error);
    }

    SUBCASE("错误报告异常测试 - FATAL") {
        // 测试 FATAL 级别异常
        CHECK_THROWS_AS(SIM_FATAL("这是一条致命错误信息"), std::runtime_error);
    }

    SUBCASE("错误类型枚举值测试") {
        // 验证错误类型的顺序
        CHECK(static_cast<int>(ErrorType::DEBUG) == 0);
        CHECK(static_cast<int>(ErrorType::INFO) == 1);
        CHECK(static_cast<int>(ErrorType::WARNING) == 2);
        CHECK(static_cast<int>(ErrorType::ERROR) == 3);
        CHECK(static_cast<int>(ErrorType::FATAL) == 4);
    }
} 