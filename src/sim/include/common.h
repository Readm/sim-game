#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <stdexcept>

// 编译选项
#define SIM_DEBUG_MODE 0
#define SIM_RELEASE_MODE 1

// 错误等级
enum class ErrorLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    FATAL = 4
};

// 当前编译模式
#ifndef SIM_BUILD_MODE
#define SIM_BUILD_MODE SIM_DEBUG_MODE
#endif

// 根据编译模式设置日志级别
#if SIM_BUILD_MODE == SIM_DEBUG_MODE
#define SIM_LOG_LEVEL ErrorLevel::DEBUG
#else
#define SIM_LOG_LEVEL ErrorLevel::INFO
#endif

namespace sim {

// 错误类型枚举
enum class ErrorType {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

// 错误处理类
class Error {
public:
    static void report(ErrorType type, const std::string& msg, const char* file, int line) {
        std::cerr << "[" << getCurrentTime() << "] ";
        switch (type) {
            case ErrorType::TRACE:   std::cerr << "TRACE: "; break;
            case ErrorType::DEBUG:   std::cerr << "DEBUG: "; break;
            case ErrorType::INFO:    std::cerr << "INFO: "; break;
            case ErrorType::WARNING: std::cerr << "WARNING: "; break;
            case ErrorType::ERROR:   std::cerr << "ERROR: "; break;
            case ErrorType::FATAL:   std::cerr << "FATAL: "; break;
        }
        std::cerr << msg << " (" << file << ":" << line << ")" << std::endl;

        if (type == ErrorType::FATAL) {
            throw std::runtime_error(msg);
        }
    }

    static const char* getErrorTypeName(ErrorType type) {
        switch (type) {
            case ErrorType::TRACE:   return "TRACE";
            case ErrorType::DEBUG:   return "DEBUG";
            case ErrorType::INFO:    return "INFO";
            case ErrorType::WARNING: return "WARNING";
            case ErrorType::ERROR:   return "ERROR";
            case ErrorType::FATAL:   return "FATAL";
            default:                 return "UNKNOWN";
        }
    }

private:
    static std::string getCurrentTime() {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

// 便捷的错误报告宏
#define SIM_TRACE(msg)   sim::Error::report(sim::ErrorType::TRACE, msg, __FILE__, __LINE__)
#define SIM_DEBUG(msg)   sim::Error::report(sim::ErrorType::DEBUG, msg, __FILE__, __LINE__)
#define SIM_INFO(msg)    sim::Error::report(sim::ErrorType::INFO, msg, __FILE__, __LINE__)
#define SIM_WARNING(msg) sim::Error::report(sim::ErrorType::WARNING, msg, __FILE__, __LINE__)
#define SIM_ERROR(msg)   sim::Error::report(sim::ErrorType::ERROR, msg, __FILE__, __LINE__)
#define SIM_FATAL(msg)   sim::Error::report(sim::ErrorType::FATAL, msg, __FILE__, __LINE__)

} // namespace sim 