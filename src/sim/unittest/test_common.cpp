#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "../include/common.h"

TEST_CASE("Common Macros") {
    using namespace sim;

    // 测试编译模式
    CHECK(SIM_BUILD_MODE == SIM_DEBUG_MODE);

    // 测试错误报告
    SIM_DEBUG("This is a debug message");
    SIM_INFO("This is an info message");
    SIM_WARNING("This is a warning message");
    SIM_ERROR("This is an error message");
}

TEST_CASE("Error Type Order") {
    using namespace sim;
    
    // 验证错误类型的顺序
    CHECK(static_cast<int>(ErrorType::FATAL) < static_cast<int>(ErrorType::ERROR));
    CHECK(static_cast<int>(ErrorType::ERROR) < static_cast<int>(ErrorType::WARNING));
    CHECK(static_cast<int>(ErrorType::WARNING) < static_cast<int>(ErrorType::INFO));
    CHECK(static_cast<int>(ErrorType::INFO) < static_cast<int>(ErrorType::DEBUG));
    CHECK(static_cast<int>(ErrorType::DEBUG) < static_cast<int>(ErrorType::TRACE));
} 