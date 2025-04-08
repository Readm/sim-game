#ifndef SIM_COMMON_H
#define SIM_COMMON_H

#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>

// 编译等级定义
#define SIM_DEBUG_LEVEL 1
#define SIM_RELEASE_LEVEL 2

// 当前编译等级
#ifndef SIM_COMPILE_LEVEL
#define SIM_COMPILE_LEVEL SIM_DEBUG_LEVEL
#endif

namespace sim {

// 错误类型枚举
enum class ErrorType {
    // 致命错误：会导致模拟器立即终止
    FATAL,          // 致命错误，无法继续执行
    PANIC,          // 内部错误，通常是bug导致

    // 运行时错误：可能影响模拟结果但不会立即终止
    ERROR,          // 严重错误，但可能可以继续执行
    WARNING,        // 警告信息，可能会影响结果准确性
    
    // 信息类消息：不影响模拟执行
    INFO,           // 一般信息
    HACK,           // 临时解决方案的提示
    
    // 调试信息：仅在调试模式下输出
    DEBUG,          // 调试信息
    TRACE           // 跟踪信息，最详细的日志级别
};

// 错误处理类
class Error {
public:
    static void report(ErrorType type, const std::string& message, 
                      const char* file = nullptr, int line = 0) {
        // 根据编译等级和日志等级决定是否输出
        if (SIM_COMPILE_LEVEL == SIM_DEBUG_LEVEL) {
            // 在调试模式下输出所有信息
            printError(type, message, file, line);
        } else {
            // 在发布模式下只输出重要信息
            if (type <= ErrorType::WARNING) {
                printError(type, message, file, line);
            }
        }

        // 对于致命错误，终止程序
        if (type == ErrorType::FATAL || type == ErrorType::PANIC) {
            std::abort();
        }
    }

private:
    static const char* getErrorTypeName(ErrorType type) {
        switch (type) {
            case ErrorType::FATAL:   return "FATAL";
            case ErrorType::PANIC:   return "PANIC";
            case ErrorType::ERROR:   return "ERROR";
            case ErrorType::WARNING: return "WARNING";
            case ErrorType::INFO:    return "INFO";
            case ErrorType::HACK:    return "HACK";
            case ErrorType::DEBUG:   return "DEBUG";
            case ErrorType::TRACE:   return "TRACE";
            default:                 return "UNKNOWN";
        }
    }

    static std::string getCurrentTime() {
        std::time_t now = std::time(nullptr);
        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return std::string(buffer);
    }

    static void printError(ErrorType type, const std::string& message,
                          const char* file, int line) {
        // 获取当前时间
        std::string time = getCurrentTime();
        
        // 构建错误消息
        std::cerr << "[" << std::setw(7) << getErrorTypeName(type) << "] "
                  << time << " - " << message;
        
        // 如果提供了文件和行号信息，则输出
        if (file != nullptr) {
            std::cerr << " (" << file;
            if (line > 0) {
                std::cerr << ":" << line;
            }
            std::cerr << ")";
        }
        
        std::cerr << std::endl;
    }
};

// 便捷的错误报告宏
#define SIM_FATAL(msg)   sim::Error::report(sim::ErrorType::FATAL, msg, __FILE__, __LINE__)
#define SIM_PANIC(msg)   sim::Error::report(sim::ErrorType::PANIC, msg, __FILE__, __LINE__)
#define SIM_ERROR(msg)   sim::Error::report(sim::ErrorType::ERROR, msg, __FILE__, __LINE__)
#define SIM_WARNING(msg) sim::Error::report(sim::ErrorType::WARNING, msg, __FILE__, __LINE__)
#define SIM_INFO(msg)    sim::Error::report(sim::ErrorType::INFO, msg, __FILE__, __LINE__)
#define SIM_HACK(msg)    sim::Error::report(sim::ErrorType::HACK, msg, __FILE__, __LINE__)
#define SIM_DEBUG(msg)   sim::Error::report(sim::ErrorType::DEBUG, msg, __FILE__, __LINE__)
#define SIM_TRACE(msg)   sim::Error::report(sim::ErrorType::TRACE, msg, __FILE__, __LINE__)

} // namespace sim

#endif // SIM_COMMON_H 