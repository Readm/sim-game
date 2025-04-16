#include "network_client.h"
#include <iostream>
#include <thread>
#include <chrono>

// 简单模拟HTTP请求实现
NetworkClient::NetworkClient(const std::string& baseUrl)
    : m_BaseUrl(baseUrl)
    , m_Connected(false)
{
}

NetworkClient::~NetworkClient()
{
    disconnect();
}

bool NetworkClient::isConnected() const
{
    return m_Connected;
}

bool NetworkClient::connect()
{
    // 模拟连接延迟
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 模拟成功连接
    m_Connected = true;
    std::cout << "已连接到服务器: " << m_BaseUrl << std::endl;
    return true;
}

void NetworkClient::disconnect()
{
    if (m_Connected)
    {
        // 模拟断开连接
        m_Connected = false;
        std::cout << "已断开与服务器的连接" << std::endl;
    }
}

bool NetworkClient::startSimulation()
{
    if (!m_Connected)
        return false;
        
    json response = post("/api/simulation/start", json());
    std::cout << "模拟已启动" << std::endl;
    return true;
}

bool NetworkClient::stopSimulation()
{
    if (!m_Connected)
        return false;
        
    json response = post("/api/simulation/stop", json());
    std::cout << "模拟已停止" << std::endl;
    return true;
}

bool NetworkClient::stepSimulation()
{
    if (!m_Connected)
        return false;
        
    json response = post("/api/simulation/step", json());
    std::cout << "模拟已步进" << std::endl;
    return true;
}

bool NetworkClient::resetSimulation()
{
    if (!m_Connected)
        return false;
        
    json response = post("/api/simulation/reset", json());
    std::cout << "模拟已重置" << std::endl;
    return true;
}

json NetworkClient::getNetworkState()
{
    if (!m_Connected)
        return json("{}");
        
    return get("/api/network/state");
}

bool NetworkClient::updateNode(int nodeId, const json& properties)
{
    if (!m_Connected)
        return false;
        
    json data = json("{\"id\":" + std::to_string(nodeId) + "}");
    json response = put("/api/network/nodes/" + std::to_string(nodeId), data);
    return true;
}

bool NetworkClient::createConnection(int sourceNodeId, int sourcePort, int targetNodeId, int targetPort)
{
    if (!m_Connected)
        return false;
        
    json data = json("{\"sourceNodeId\":" + std::to_string(sourceNodeId) + 
                     ",\"sourcePort\":" + std::to_string(sourcePort) + 
                     ",\"targetNodeId\":" + std::to_string(targetNodeId) + 
                     ",\"targetPort\":" + std::to_string(targetPort) + "}");
    
    json response = post("/api/network/connections", data);
    return true;
}

bool NetworkClient::deleteConnection(int connectionId)
{
    if (!m_Connected)
        return false;
        
    json response = del("/api/network/connections/" + std::to_string(connectionId));
    return true;
}

void NetworkClient::setStateUpdateCallback(std::function<void(const json&)> callback)
{
    m_StateUpdateCallback = std::move(callback);
}

// 模拟HTTP请求
json NetworkClient::get(const std::string& endpoint)
{
    std::cout << "GET " << m_BaseUrl << endpoint << std::endl;
    // 模拟网络延迟
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return json("{\"status\":\"success\"}");
}

json NetworkClient::post(const std::string& endpoint, const json& data)
{
    std::cout << "POST " << m_BaseUrl << endpoint << " 数据: " << data.dump() << std::endl;
    // 模拟网络延迟
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return json("{\"status\":\"success\"}");
}

json NetworkClient::put(const std::string& endpoint, const json& data)
{
    std::cout << "PUT " << m_BaseUrl << endpoint << " 数据: " << data.dump() << std::endl;
    // 模拟网络延迟
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return json("{\"status\":\"success\"}");
}

json NetworkClient::del(const std::string& endpoint)
{
    std::cout << "DELETE " << m_BaseUrl << endpoint << std::endl;
    // 模拟网络延迟
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return json("{\"status\":\"success\"}");
} 