#pragma once

#include <string>
#include <functional>
#include <future>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

// 使用nlohmann/json库
using json = nlohmann::json;

class NetworkClient {
public:
    NetworkClient(const std::string& baseUrl = "http://localhost:8080");
    ~NetworkClient();

    // 连接状态
    bool isConnected() const;
    bool connect();
    void disconnect();
    
    // 模拟控制API
    bool startSimulation();
    bool stopSimulation();
    bool stepSimulation();
    bool resetSimulation();
    
    // 网络状态API
    json getNetworkState();
    bool updateNode(int nodeId, const json& properties);
    bool createConnection(int sourceNodeId, int sourcePort, int targetNodeId, int targetPort);
    bool deleteConnection(int connectionId);
    
    // 设置回调
    void setStateUpdateCallback(std::function<void(const json&)> callback);
    
private:
    // HTTP请求实现
    json get(const std::string& endpoint);
    json post(const std::string& endpoint, const json& data);
    json put(const std::string& endpoint, const json& data);
    json del(const std::string& endpoint);
    
    std::string m_BaseUrl;
    bool m_Connected;
    std::function<void(const json&)> m_StateUpdateCallback;
}; 