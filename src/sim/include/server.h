/**
 * @file server.h
 * @brief 定义了仿真系统中的HTTP服务器和仿真引擎
 * 
 * 该文件实现了仿真系统的Web服务接口：
 * - Server类：提供HTTP API接口，用于控制仿真和获取状态
 * - SimulationEngine类：负责实际的网络仿真计算
 * 
 * 服务器功能：
 * - RESTful API接口
 * - 网络配置的加载和管理
 * - 仿真控制（启动、停止、单步执行、重置）
 * - 实时状态监控和广播
 * - 请求日志记录
 * - 多线程安全的状态管理
 * 
 * 仿真引擎功能：
 * - 独立的仿真计算线程
 * - 网络状态的实时更新
 * - 支持多种仿真模式
 * - 线程安全的状态访问
 */

#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include "network.h"
#include <httplib.h>

// Using nlohmann::json library
using json = nlohmann::json;

namespace sim {

// Forward declaration
class SimulationEngine;

/**
 * @brief 网络仿真服务器
 * 
 * 提供HTTP接口用于控制网络仿真和获取状态更新
 */
class Server {
public:
    /**
     * @brief 构造服务器
     * @param port 服务器端口号
     */
    Server(int port = 8080);
    
    /**
     * @brief 析构函数
     */
    virtual ~Server();
    
    /**
     * @brief 启动服务器
     * @return 是否启动成功
     */
    bool start();
    
    /**
     * @brief 停止服务器
     */
    void stop();
    
    /**
     * @brief 检查服务器是否正在运行
     * @return 运行状态
     */
    bool isRunning() const;
    
    /**
     * @brief 从文件加载网络配置
     * @param filepath JSON文件路径
     * @return 是否加载成功
     */
    bool loadNetworkFromFile(const std::string& filepath);
    
    /**
     * @brief 从JSON字符串加载网络配置
     * @param jsonStr JSON字符串
     * @return 是否加载成功
     */
    bool loadNetworkFromJson(const std::string& jsonStr);
    
    /**
     * @brief 获取当前网络状态
     * @return 网络状态JSON
     */
    json getNetworkState() const;
    
    /**
     * @brief 启动仿真
     * @return 是否启动成功
     */
    bool startSimulation();
    
    /**
     * @brief 停止仿真
     * @return 是否停止成功
     */
    bool stopSimulation();
    
    /**
     * @brief 执行仿真步骤
     * @return 是否执行成功
     */
    bool stepSimulation();
    
    /**
     * @brief 重置仿真
     * @return 是否重置成功
     */
    bool resetSimulation();
    
    /**
     * @brief 安全停止服务器
     * @return 是否停止成功
     */
    bool shutdown();
    
    /**
     * @brief 设置状态更新回调
     * @param callback 回调函数
     */
    void setStateUpdateCallback(std::function<void(const json&)> callback);
    
    /**
     * @brief 广播状态更新
     * @param state 状态JSON
     */
    void broadcastStateUpdate(const json& state);
    
    /**
     * @brief 获取请求历史（用于测试）
     * @return 请求日志
     */
    std::vector<std::string> getRequestLog() const;
    
    /**
     * @brief 清除请求历史
     */
    void clearRequestLog();
    
    /**
     * @brief 创建生产者-消费者网络
     * @return 是否创建成功
     */
    bool createProducerConsumerNetwork();

private:
    /**
     * @brief 初始化HTTP路由
     */
    void initHttpRoutes();
    
    /**
     * @brief 更新网络状态
     */
    void updateNetworkState();
    
    int m_Port;
    std::atomic<bool> m_Running;
    
    // HTTP server
    std::unique_ptr<httplib::Server> m_HttpServer;
    std::thread m_ServerThread;
    
    // Use mutex to protect state access
    mutable std::mutex m_StateMutex;
    json m_NetworkState;
    
    // Simulation engine
    std::unique_ptr<SimulationEngine> m_SimEngine;
    
    // Request log
    mutable std::mutex m_LogMutex;
    std::vector<std::string> m_RequestLog;
    
    // State update callback
    std::function<void(const json&)> m_StateUpdateCallback;
};

/**
 * @brief 仿真引擎类
 * 
 * 负责实际的网络仿真计算，服务器只提供API接口
 */
class SimulationEngine {
public:
    /**
     * @brief 构造函数
     */
    SimulationEngine();
    
    /**
     * @brief 析构函数
     */
    ~SimulationEngine();
    
    /**
     * @brief 从文件加载网络
     * @param filepath 文件路径
     * @return 是否加载成功
     */
    bool loadFromFile(const std::string& filepath);
    
    /**
     * @brief 从JSON字符串加载网络
     * @param jsonStr JSON字符串
     * @return 是否加载成功
     */
    bool loadFromJson(const std::string& jsonStr);
    
    /**
     * @brief 创建预定义网络
     * @return 是否创建成功
     */
    bool createProducerConsumerNetwork();
    
    /**
     * @brief 启动仿真
     * @return 是否启动成功
     */
    bool start();
    
    /**
     * @brief 停止仿真
     * @return 是否停止成功
     */
    bool stop();
    
    /**
     * @brief 单步执行仿真
     * @return 是否执行成功
     */
    bool step();
    
    /**
     * @brief 重置仿真
     * @return 是否重置成功
     */
    bool reset();
    
    /**
     * @brief 获取状态
     * @return 状态JSON
     */
    json getState() const;
    
    /**
     * @brief 检查是否正在运行
     * @return 运行状态
     */
    bool isRunning() const;
    
    /**
     * @brief 获取当前时钟周期
     * @return 当前时钟周期
     */
    int getCurrentTick() const;

private:
    /**
     * @brief 仿真线程函数
     */
    void simulationThread();
    
    /**
     * @brief 更新网络状态
     */
    void updateNetworkState();
    
    std::atomic<bool> m_Running;
    std::atomic<bool> m_ShouldRun;
    std::thread m_SimThread;
    
    int m_CurrentTick;
    mutable std::mutex m_StateMutex;
    json m_NetworkState;
    
    // Simulation network
    std::shared_ptr<Network> m_Network;
};

} // namespace sim 