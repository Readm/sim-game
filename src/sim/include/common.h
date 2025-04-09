#pragma once

#include <string>
#include <iostream>

namespace sim {

// 错误类型枚举
enum class ErrorType {
    DEBUG,   // 调试信息
    INFO,    // 普通信息
    WARNING, // 警告信息
    ERROR,   // 错误信息，模型错误信息
    FATAL     // 致命错误信息，框架错误信息
};

// 简单的错误报告函数
inline void reportError(ErrorType type, const std::string& msg) {
    switch (type) {
        case ErrorType::DEBUG:   std::cerr << "DEBUG: "; break;
        case ErrorType::INFO:    std::cerr << "INFO: "; break;
        case ErrorType::WARNING: std::cerr << "WARNING: "; break;
        case ErrorType::ERROR:   std::cerr << "ERROR: "; break;
        case ErrorType::FATAL:   std::cerr << "FATAL: "; break;
    }
    std::cerr << msg << std::endl;
}

// 便捷的错误报告宏
#define SIM_DEBUG(msg)   sim::reportError(sim::ErrorType::DEBUG, msg)
#define SIM_INFO(msg)    sim::reportError(sim::ErrorType::INFO, msg)
#define SIM_WARNING(msg) sim::reportError(sim::ErrorType::WARNING, msg)
#define SIM_ERROR(msg)   sim::reportError(sim::ErrorType::ERROR, msg)
#define SIM_FATAL(msg)   sim::reportError(sim::ErrorType::FATAL, msg)

} // namespace sim 