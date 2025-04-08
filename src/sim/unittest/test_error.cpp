#include "doctest/doctest.h"
#include "../include/common.h"
#include <sstream>
#include <string>

TEST_CASE("测试错误处理系统") {
    using namespace sim;

    SUBCASE("测试错误类型") {
        // 验证错误类型的顺序
        CHECK(static_cast<int>(ErrorType::FATAL) < static_cast<int>(ErrorType::ERROR));
        CHECK(static_cast<int>(ErrorType::ERROR) < static_cast<int>(ErrorType::WARNING));
        CHECK(static_cast<int>(ErrorType::WARNING) < static_cast<int>(ErrorType::INFO));
        CHECK(static_cast<int>(ErrorType::INFO) < static_cast<int>(ErrorType::DEBUG));
    }
    
    SUBCASE("测试非致命错误") {
        // 非致命错误不应该导致程序终止
        Error::report(ErrorType::WARNING, "This is a test warning");
        Error::report(ErrorType::INFO, "This is a test info");
        Error::report(ErrorType::DEBUG, "This is a test debug message");
        CHECK(true); // 如果能执行到这里，说明程序没有终止
    }
    
    SUBCASE("测试错误宏") {
        // 测试各种错误报告宏
        SIM_DEBUG_LOG("Debug message");
        SIM_INFO("Info message");
        SIM_WARNING("Warning message");
        SIM_ERROR("Error message");
        SIM_HACK("Hack message");
        SIM_TRACE("Trace message");
        CHECK(true); // 如果能执行到这里，说明非致命错误宏工作正常
    }

    SUBCASE("测试错误输出格式") {
        MESSAGE("以下输出应该包含时间戳、错误类型和位置信息：");
        
        // 测试不同类型的错误输出
        Error::report(ErrorType::WARNING, "Test warning message", "test.cpp", 42);
        Error::report(ErrorType::INFO, "Test info message");
        Error::report(ErrorType::DEBUG, "Test debug message", "debug.cpp", 100);
        
        // 注意：由于输出到stderr，我们无法直接捕获和验证输出
        // 但可以通过肉眼观察输出格式是否符合预期：
        MESSAGE("请检查上述输出是否符合以下格式：");
        MESSAGE("[错误类型] YYYY-MM-DD HH:MM:SS - 消息内容 (文件名:行号)");
    }

    SUBCASE("Error Reporting") {
        // 测试不同级别的错误报告
        SIM_DEBUG_LOG("Debug message");
        SIM_INFO("Info message");
        SIM_WARNING("Warning message");
        SIM_ERROR("Error message");
        SIM_HACK("Hack message");
        SIM_TRACE("Trace message");

        // 测试错误类型名称
        CHECK(std::string(Error::getErrorTypeName(ErrorType::DEBUG)) == "DEBUG");
        CHECK(std::string(Error::getErrorTypeName(ErrorType::INFO)) == "INFO");
        CHECK(std::string(Error::getErrorTypeName(ErrorType::WARNING)) == "WARNING");
        CHECK(std::string(Error::getErrorTypeName(ErrorType::ERROR)) == "ERROR");
        CHECK(std::string(Error::getErrorTypeName(ErrorType::FATAL)) == "FATAL");
        CHECK(std::string(Error::getErrorTypeName(ErrorType::PANIC)) == "PANIC");
        CHECK(std::string(Error::getErrorTypeName(ErrorType::HACK)) == "HACK");
        CHECK(std::string(Error::getErrorTypeName(ErrorType::TRACE)) == "TRACE");
    }
} 