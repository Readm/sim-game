#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include "network.h"

// 简单的JSON类，与NetworkClient中的兼容
class json {
public:
    json() = default;
    json(const std::string& str) : m_Data(str) {}
    
    static json array() { return json("[]"); }
    static json object() { return json("{}"); }
    
    std::string dump() const { return m_Data; }
    
    bool operator==(bool value) const { return (m_Data == "true") == value; }
    bool operator==(int value) const { return std::stoi(m_Data) == value; }
    bool operator!=(const json& other) const { return m_Data != other.m_Data; }
    
    int get_int() const { return std::stoi(m_Data); }
    
    json& operator[](const std::string& key) {
        m_Data += "." + key;
        return *this;
    }
    
    json& operator[](size_t index) {
        m_Data += "[" + std::to_string(index) + "]";
        return *this;
    }
    
    size_t size() const { return 1; }
    
private:
    std::string m_Data;
};

namespace sim {

// 前向声明
class SimulationEngine;

/**
 * @brief 网络模拟服务器
 * 
 * 提供HTTP和WebSocket接口，用于控制网络模拟并获取状态更新
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
     * @return 是否成功启动
     */
    bool start();
    
    /**
     * @brief 停止服务器
     */
    void stop();
    
    /**
     * @brief 检查服务器是否运行中
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
     * @brief 启动模拟
     * @return 是否成功启动
     */
    bool startSimulation();
    
    /**
     * @brief 停止模拟
     * @return 是否成功停止
     */
    bool stopSimulation();
    
    /**
     * @brief 单步执行模拟
     * @return 是否成功执行
     */
    bool stepSimulation();
    
    /**
     * @brief 重置模拟
     * @return 是否成功重置
     */
    bool resetSimulation();
    
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
     * @brief 获取请求历史(用于测试)
     * @return 请求日志
     */
    std::vector<std::string> getRequestLog() const;
    
    /**
     * @brief 清空请求历史
     */
    void clearRequestLog();
    
    /**
     * @brief 处理API请求
     * @param method HTTP方法
     * @param path 请求路径
     * @param body 请求体
     * @return 响应内容
     */
    std::string processApiRequest(const std::string& method, const std::string& path, const std::string& body = "");
    
    /**
     * @brief 创建生产者-消费者网络
     * @return 是否成功创建
     */
    bool createProducerConsumerNetwork();

private:
    // 服务器线程函数
    void serverThread();
    
    // 处理HTTP请求
    std::string handleRequest(const std::string& method, const std::string& path, const std::string& body);
    
    // 更新网络状态
    void updateNetworkState();
    
    int m_Port;
    std::atomic<bool> m_Running;
    std::thread m_ServerThread;
    
    // 使用互斥锁保护状态访问
    mutable std::mutex m_StateMutex;
    json m_NetworkState;
    
    // 模拟引擎
    std::unique_ptr<SimulationEngine> m_SimEngine;
    
    // 请求日志
    mutable std::mutex m_LogMutex;
    std::vector<std::string> m_RequestLog;
    
    // 状态更新回调
    std::function<void(const json&)> m_StateUpdateCallback;
};

/**
 * @brief 模拟引擎类
 * 
 * 负责实际的网络模拟计算，服务器仅提供API接口
 */
class SimulationEngine {
public:
    SimulationEngine();
    ~SimulationEngine();
    
    // 加载网络
    bool loadFromFile(const std::string& filepath);
    bool loadFromJson(const std::string& jsonStr);
    
    // 创建预定义网络
    bool createProducerConsumerNetwork();
    
    // 控制模拟
    bool start();
    bool stop();
    bool step();
    bool reset();
    
    // 获取状态
    json getState() const;
    bool isRunning() const;
    int getCurrentTick() const;

private:
    // 模拟线程函数
    void simulationThread();
    
    // 更新网络状态
    void updateNetworkState();
    
    std::atomic<bool> m_Running;
    std::atomic<bool> m_ShouldRun;
    std::thread m_SimThread;
    
    int m_CurrentTick;
    mutable std::mutex m_StateMutex;
    json m_NetworkState;
    
    // 仿真网络
    std::shared_ptr<Network> m_Network;
};

} // namespace sim 