#pragma once

#include <string>
#include <functional>
#include <future>
#include <vector>
#include <memory>

// 模拟JSON库
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