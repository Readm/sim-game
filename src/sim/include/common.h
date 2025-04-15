/**
 * @file common.h
 * @brief 定义了仿真系统中的通用工具和错误处理机制
 * 
 * 该文件提供：
 * - 错误类型枚举（DEBUG、INFO、WARNING、ERROR、FATAL）
 * - 错误报告函数
 * - 便捷的错误报告宏
 * 
 * 错误处理机制支持不同级别的错误报告，从调试信息到致命错误，
 * 并在适当的情况下抛出异常。
 */

#pragma once

#include <string>
#include <iostream>
#include <stdexcept>

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
        case ErrorType::DEBUG:   
            std::cerr << "DEBUG: " << msg << std::endl;
            break;
        case ErrorType::INFO:    
            std::cerr << "INFO: " << msg << std::endl;
            break;
        case ErrorType::WARNING: 
            std::cerr << "WARNING: " << msg << std::endl;
            break;
        case ErrorType::ERROR:   
            std::cerr << "ERROR: " << msg << std::endl;
            throw std::runtime_error("ERROR: " + msg);
        case ErrorType::FATAL:   
            std::cerr << "FATAL: " << msg << std::endl;
            throw std::runtime_error("FATAL: " + msg);
    }
}

// 便捷的错误报告宏
#define SIM_DEBUG(msg)   sim::reportError(sim::ErrorType::DEBUG, msg)
#define SIM_INFO(msg)    sim::reportError(sim::ErrorType::INFO, msg)
#define SIM_WARNING(msg) sim::reportError(sim::ErrorType::WARNING, msg)
#define SIM_ERROR(msg)   sim::reportError(sim::ErrorType::ERROR, msg)
#define SIM_FATAL(msg)   sim::reportError(sim::ErrorType::FATAL, msg)

} // namespace sim 