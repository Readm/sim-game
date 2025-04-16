#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <unordered_map>
#include "../network_client.h" // 使用我们定义的简单json库

// 模拟服务器类，用于测试前端与后端的交互
class MockServer {
public:
    MockServer(int port = 8080);
    virtual ~MockServer();

    // 服务器控制
    bool start();
    void stop();
    bool isRunning() const;
    
    // 模拟网络状态
    void setNetworkState(const json& state);
    json getNetworkState() const;
    
    // 模拟延迟和错误
    void setResponseDelay(int milliseconds);
    void setErrorRate(float rate); // 0.0-1.0
    
    // 获取请求历史记录（用于测试验证）
    std::vector<std::string> getRequestLog() const;
    void clearRequestLog();
    
    // 设置状态更新回调
    void setStateUpdateCallback(std::function<void(const json&)> callback) {
        m_StateUpdateCallback = std::move(callback);
    }
    
    // 广播状态更新
    void broadcastStateUpdate(const json& state) {
        if (m_StateUpdateCallback) {
            m_StateUpdateCallback(state);
        }
    }

private:
    // 内部HTTP服务器实现
    void serverThread();
    std::string handleRequest(const std::string& method, const std::string& path, const std::string& body);
    
    int m_Port;
    std::atomic<bool> m_Running;
    std::thread m_ServerThread;
    
    mutable std::mutex m_StateMutex;
    json m_NetworkState;
    
    int m_ResponseDelay = 0;
    float m_ErrorRate = 0.0f;
    
    mutable std::mutex m_LogMutex;
    std::vector<std::string> m_RequestLog;
    
    std::function<void(const json&)> m_StateUpdateCallback;
}; 