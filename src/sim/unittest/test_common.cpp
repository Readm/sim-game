#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../include/common.h"

TEST_CASE("测试common.h中的宏定义") {
    SUBCASE("测试编译等级") {
        CHECK(SIM_COMPILE_LEVEL == SIM_DEBUG_LEVEL);
    }
    
    SUBCASE("测试错误类型") {
        // 验证错误类型的顺序
        CHECK(static_cast<int>(sim::ErrorType::FATAL) < static_cast<int>(sim::ErrorType::ERROR));
        CHECK(static_cast<int>(sim::ErrorType::ERROR) < static_cast<int>(sim::ErrorType::WARNING));
        CHECK(static_cast<int>(sim::ErrorType::WARNING) < static_cast<int>(sim::ErrorType::INFO));
        CHECK(static_cast<int>(sim::ErrorType::INFO) < static_cast<int>(sim::ErrorType::DEBUG));
    }
} 