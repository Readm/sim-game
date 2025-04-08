#include "doctest.h"
#include "../include/common.h"
#include <sstream>

TEST_CASE("测试错误处理系统") {
    SUBCASE("测试错误类型") {
        // 验证错误类型的顺序
        CHECK(static_cast<int>(sim::ErrorType::FATAL) < static_cast<int>(sim::ErrorType::ERROR));
        CHECK(static_cast<int>(sim::ErrorType::ERROR) < static_cast<int>(sim::ErrorType::WARNING));
        CHECK(static_cast<int>(sim::ErrorType::WARNING) < static_cast<int>(sim::ErrorType::INFO));
        CHECK(static_cast<int>(sim::ErrorType::INFO) < static_cast<int>(sim::ErrorType::DEBUG));
    }
    
    SUBCASE("测试非致命错误") {
        // 非致命错误不应该导致程序终止
        sim::Error::report(sim::ErrorType::WARNING, "This is a test warning");
        sim::Error::report(sim::ErrorType::INFO, "This is a test info");
        sim::Error::report(sim::ErrorType::DEBUG, "This is a test debug message");
        CHECK(true); // 如果能执行到这里，说明程序没有终止
    }
    
    SUBCASE("测试错误宏") {
        // 测试各种错误报告宏
        SIM_WARNING("Test warning macro");
        SIM_INFO("Test info macro");
        SIM_DEBUG("Test debug macro");
        SIM_HACK("Test hack macro");
        SIM_TRACE("Test trace macro");
        CHECK(true); // 如果能执行到这里，说明宏正常工作
    }

    SUBCASE("测试错误输出格式") {
        MESSAGE("以下输出应该包含时间戳、错误类型和位置信息：");
        
        // 测试不同类型的错误输出
        sim::Error::report(sim::ErrorType::WARNING, "Test warning message", "test.cpp", 42);
        sim::Error::report(sim::ErrorType::INFO, "Test info message");
        sim::Error::report(sim::ErrorType::DEBUG, "Test debug message", "debug.cpp", 100);
        
        // 注意：由于输出到stderr，我们无法直接捕获和验证输出
        // 但是可以通过目视检查输出格式是否正确
        MESSAGE("请检查上述输出是否符合以下格式：");
        MESSAGE("[错误类型] YYYY-MM-DD HH:MM:SS - 消息内容 (文件名:行号)");
    }
} 