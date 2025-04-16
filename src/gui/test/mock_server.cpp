#include "mock_server.h"
#include <iostream>
#include <random>
#include <sstream>

MockServer::MockServer(int port)
    : m_Port(port)
    , m_Running(false)
    , m_NetworkState(json("{\"tick\":0,\"running\":false,\"nodes\":[],\"connections\":[]}"))
{
}

MockServer::~MockServer()
{
    stop();
}

bool MockServer::start()
{
    if (m_Running)
        return true;
        
    m_Running = true;
    m_ServerThread = std::thread(&MockServer::serverThread, this);
    
    std::cout << "模拟服务器已启动，端口: " << m_Port << std::endl;
    return true;
}

void MockServer::stop()
{
    if (!m_Running)
        return;
        
    m_Running = false;
    if (m_ServerThread.joinable())
        m_ServerThread.join();
    
    std::cout << "模拟服务器已停止" << std::endl;
}

bool MockServer::isRunning() const
{
    return m_Running;
}

void MockServer::setNetworkState(const json& state)
{
    std::lock_guard<std::mutex> lock(m_StateMutex);
    m_NetworkState = state;
}

json MockServer::getNetworkState() const
{
    std::lock_guard<std::mutex> lock(m_StateMutex);
    return m_NetworkState;
}

void MockServer::setResponseDelay(int milliseconds)
{
    m_ResponseDelay = milliseconds;
}

void MockServer::setErrorRate(float rate)
{
    m_ErrorRate = std::max(0.0f, std::min(1.0f, rate));
}

std::vector<std::string> MockServer::getRequestLog() const
{
    std::lock_guard<std::mutex> lock(m_LogMutex);
    return m_RequestLog;
}

void MockServer::clearRequestLog()
{
    std::lock_guard<std::mutex> lock(m_LogMutex);
    m_RequestLog.clear();
}

void MockServer::serverThread()
{
    std::cout << "服务器线程已启动" << std::endl;
    
    while (m_Running)
    {
        // 模拟处理请求
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "服务器线程已退出" << std::endl;
}

std::string MockServer::handleRequest(const std::string& method, const std::string& path, const std::string& body)
{
    // 添加到请求日志
    {
        std::lock_guard<std::mutex> lock(m_LogMutex);
        std::stringstream ss;
        ss << method << " " << path << " " << body;
        m_RequestLog.push_back(ss.str());
    }
    
    // 模拟响应延迟
    if (m_ResponseDelay > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(m_ResponseDelay));
    
    // 模拟错误
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    if (dis(gen) < m_ErrorRate)
    {
        return "{\"status\":\"error\",\"message\":\"模拟错误\"}";
    }
    
    // 处理不同的API请求
    if (path == "/api/status")
    {
        return "{\"status\":\"ok\",\"version\":\"1.0\"}";
    }
    else if (path == "/api/simulation/start")
    {
        std::lock_guard<std::mutex> lock(m_StateMutex);
        json state = m_NetworkState;
        // 更新状态
        state = json("{\"tick\":0,\"running\":true,\"nodes\":[],\"connections\":[]}");
        m_NetworkState = state;
        return "{\"status\":\"ok\"}";
    }
    else if (path == "/api/simulation/stop")
    {
        std::lock_guard<std::mutex> lock(m_StateMutex);
        json state = m_NetworkState;
        // 更新状态
        state = json("{\"tick\":0,\"running\":false,\"nodes\":[],\"connections\":[]}");
        m_NetworkState = state;
        return "{\"status\":\"ok\"}";
    }
    else if (path == "/api/network/state")
    {
        std::lock_guard<std::mutex> lock(m_StateMutex);
        return m_NetworkState.dump();
    }
    
    return "{\"status\":\"error\",\"message\":\"Unknown endpoint\"}";
} 