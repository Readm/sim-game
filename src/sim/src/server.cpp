#include "../include/server.h"
#include "../include/network_factory.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <random>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace sim {

// Server类实现

Server::Server(int port)
    : m_Port(port)
    , m_Running(false)
    , m_NetworkState(json("{\"tick\":0,\"running\":false,\"nodes\":[],\"connections\":[]}"))
    , m_SimEngine(std::make_unique<SimulationEngine>())
{
}

Server::~Server()
{
    stop();
}

bool Server::start()
{
    if (m_Running) {
        return true;
    }
    
    m_Running = true;
    m_ServerThread = std::thread(&Server::serverThread, this);
    
    std::cout << "服务器已启动，端口: " << m_Port << std::endl;
    return true;
}

void Server::stop()
{
    if (!m_Running) {
        return;
    }
    
    // 先停止模拟
    stopSimulation();
    
    // 然后停止服务器
    m_Running = false;
    if (m_ServerThread.joinable()) {
        m_ServerThread.join();
    }
    
    std::cout << "服务器已停止" << std::endl;
}

bool Server::isRunning() const
{
    return m_Running;
}

bool Server::loadNetworkFromFile(const std::string& filepath)
{
    // 检查文件是否存在
    if (!std::filesystem::exists(filepath)) {
        std::cerr << "文件不存在: " << filepath << std::endl;
        return false;
    }
    
    // 读取文件内容
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filepath << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonStr = buffer.str();
    
    return loadNetworkFromJson(jsonStr);
}

bool Server::loadNetworkFromJson(const std::string& jsonStr)
{
    // 将JSON加载到模拟引擎
    bool success = m_SimEngine->loadFromJson(jsonStr);
    
    if (success) {
        // 更新本地网络状态缓存
        std::lock_guard<std::mutex> lock(m_StateMutex);
        m_NetworkState = m_SimEngine->getState();
        
        // 记录请求
        {
            std::lock_guard<std::mutex> logLock(m_LogMutex);
            m_RequestLog.push_back("LOAD_NETWORK " + jsonStr);
        }
        
        // 广播状态更新
        broadcastStateUpdate(m_NetworkState);
    }
    
    return success;
}

json Server::getNetworkState() const
{
    std::lock_guard<std::mutex> lock(m_StateMutex);
    return m_NetworkState;
}

bool Server::startSimulation()
{
    bool success = m_SimEngine->start();
    
    if (success) {
        // 更新状态
        updateNetworkState();
        
        // 记录请求
        {
            std::lock_guard<std::mutex> lock(m_LogMutex);
            m_RequestLog.push_back("START_SIMULATION");
        }
    }
    
    return success;
}

bool Server::stopSimulation()
{
    bool success = m_SimEngine->stop();
    
    if (success) {
        // 更新状态
        updateNetworkState();
        
        // 记录请求
        {
            std::lock_guard<std::mutex> lock(m_LogMutex);
            m_RequestLog.push_back("STOP_SIMULATION");
        }
    }
    
    return success;
}

bool Server::stepSimulation()
{
    bool success = m_SimEngine->step();
    
    if (success) {
        // 更新状态
        updateNetworkState();
        
        // 记录请求
        {
            std::lock_guard<std::mutex> lock(m_LogMutex);
            m_RequestLog.push_back("STEP_SIMULATION");
        }
    }
    
    return success;
}

bool Server::resetSimulation()
{
    bool success = m_SimEngine->reset();
    
    if (success) {
        // 更新状态
        updateNetworkState();
        
        // 记录请求
        {
            std::lock_guard<std::mutex> lock(m_LogMutex);
            m_RequestLog.push_back("RESET_SIMULATION");
        }
    }
    
    return success;
}

void Server::setStateUpdateCallback(std::function<void(const json&)> callback)
{
    m_StateUpdateCallback = std::move(callback);
}

void Server::broadcastStateUpdate(const json& state)
{
    if (m_StateUpdateCallback) {
        m_StateUpdateCallback(state);
    }
}

std::vector<std::string> Server::getRequestLog() const
{
    std::lock_guard<std::mutex> lock(m_LogMutex);
    return m_RequestLog;
}

void Server::clearRequestLog()
{
    std::lock_guard<std::mutex> lock(m_LogMutex);
    m_RequestLog.clear();
}

void Server::serverThread()
{
    std::cout << "服务器线程已启动" << std::endl;
    
    // 定期检查状态变化并广播更新
    auto lastUpdate = std::chrono::steady_clock::now();
    
    while (m_Running)
    {
        // 每隔一段时间检查和广播状态更新
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count() >= 100) {
            updateNetworkState();
            lastUpdate = now;
        }
        
        // 处理请求(在真实实现中，这里应该有HTTP服务器逻辑)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "服务器线程已退出" << std::endl;
}

std::string Server::handleRequest(const std::string& method, const std::string& path, const std::string& body)
{
    // 添加到请求日志
    {
        std::lock_guard<std::mutex> lock(m_LogMutex);
        std::stringstream ss;
        ss << method << " " << path << " " << body;
        m_RequestLog.push_back(ss.str());
    }
    
    // 处理不同的API请求
    if (path == "/api/status") {
        return "{\"status\":\"ok\",\"version\":\"1.0\"}";
    }
    else if (path == "/api/simulation/start") {
        bool success = startSimulation();
        return success ? "{\"status\":\"ok\"}" : "{\"status\":\"error\",\"message\":\"无法启动模拟\"}";
    }
    else if (path == "/api/simulation/stop") {
        bool success = stopSimulation();
        return success ? "{\"status\":\"ok\"}" : "{\"status\":\"error\",\"message\":\"无法停止模拟\"}";
    }
    else if (path == "/api/simulation/step") {
        bool success = stepSimulation();
        return success ? "{\"status\":\"ok\"}" : "{\"status\":\"error\",\"message\":\"无法步进模拟\"}";
    }
    else if (path == "/api/simulation/reset") {
        bool success = resetSimulation();
        return success ? "{\"status\":\"ok\"}" : "{\"status\":\"error\",\"message\":\"无法重置模拟\"}";
    }
    else if (path == "/api/network/state") {
        std::lock_guard<std::mutex> lock(m_StateMutex);
        return m_NetworkState.dump();
    }
    else if (path == "/api/network/load" && method == "POST") {
        bool success = loadNetworkFromJson(body);
        return success ? "{\"status\":\"ok\"}" : "{\"status\":\"error\",\"message\":\"无法加载网络\"}";
    }
    else if (path == "/api/network/create/producer-consumer" && method == "POST") {
        bool success = m_SimEngine->createProducerConsumerNetwork();
        if (success) {
            updateNetworkState();
        }
        return success ? "{\"status\":\"ok\"}" : "{\"status\":\"error\",\"message\":\"无法创建生产者-消费者网络\"}";
    }
    
    return "{\"status\":\"error\",\"message\":\"未知端点\"}";
}

void Server::updateNetworkState()
{
    // 从模拟引擎获取最新状态
    json newState = m_SimEngine->getState();
    
    // 检查状态是否变化
    {
        std::lock_guard<std::mutex> lock(m_StateMutex);
        if (newState != m_NetworkState) {
            m_NetworkState = newState;
            
            // 广播状态更新
            broadcastStateUpdate(m_NetworkState);
        }
    }
}

std::string Server::processApiRequest(const std::string& method, const std::string& path, const std::string& body)
{
    return handleRequest(method, path, body);
}

bool Server::createProducerConsumerNetwork()
{
    bool success = m_SimEngine->createProducerConsumerNetwork();
    
    if (success) {
        // 更新状态
        updateNetworkState();
        
        // 记录请求
        {
            std::lock_guard<std::mutex> lock(m_LogMutex);
            m_RequestLog.push_back("CREATE_PRODUCER_CONSUMER_NETWORK");
        }
    }
    
    return success;
}

// SimulationEngine类实现

SimulationEngine::SimulationEngine()
    : m_Running(false)
    , m_ShouldRun(false)
    , m_CurrentTick(0)
    , m_NetworkState(json("{\"tick\":0,\"running\":false,\"nodes\":[],\"connections\":[]}"))
{
}

SimulationEngine::~SimulationEngine()
{
    stop();
}

bool SimulationEngine::loadFromFile(const std::string& filepath)
{
    // 检查文件是否存在
    if (!std::filesystem::exists(filepath)) {
        std::cerr << "文件不存在: " << filepath << std::endl;
        return false;
    }
    
    // 读取文件内容
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filepath << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return loadFromJson(buffer.str());
}

bool SimulationEngine::loadFromJson(const std::string& jsonStr)
{
    // 检查当前是否正在模拟
    if (m_Running) {
        std::cerr << "无法在模拟运行时加载网络" << std::endl;
        return false;
    }
    
    try {
        // 反序列化网络
        m_Network = NetworkFactory::deserializeNetwork(jsonStr);
        
        // 重置状态
        m_CurrentTick = 0;
        
        {
            std::lock_guard<std::mutex> lock(m_StateMutex);
            // 更新网络状态
            updateNetworkState();
        }
        
        std::cout << "成功加载网络配置" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "加载网络失败: " << e.what() << std::endl;
        return false;
    }
}

bool SimulationEngine::createProducerConsumerNetwork()
{
    // 检查当前是否正在模拟
    if (m_Running) {
        std::cerr << "无法在模拟运行时创建网络" << std::endl;
        return false;
    }
    
    try {
        // 创建生产者-消费者网络
        m_Network = NetworkFactory::createProducerConsumerNetwork();
        
        // 重置状态
        m_CurrentTick = 0;
        
        {
            std::lock_guard<std::mutex> lock(m_StateMutex);
            // 更新网络状态
            updateNetworkState();
        }
        
        std::cout << "成功创建生产者-消费者网络" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "创建网络失败: " << e.what() << std::endl;
        return false;
    }
}

bool SimulationEngine::start()
{
    if (m_Running) {
        return true; // 已经运行中
    }
    
    // 确保网络已加载
    if (!m_Network) {
        std::cerr << "无法启动模拟：网络未加载" << std::endl;
        return false;
    }
    
    m_ShouldRun = true;
    m_Running = true;
    m_SimThread = std::thread(&SimulationEngine::simulationThread, this);
    
    std::cout << "模拟已启动" << std::endl;
    return true;
}

bool SimulationEngine::stop()
{
    if (!m_Running) {
        return true; // 已经停止
    }
    
    m_ShouldRun = false;
    
    if (m_SimThread.joinable()) {
        m_SimThread.join();
    }
    
    m_Running = false;
    
    std::cout << "模拟已停止" << std::endl;
    
    // 更新状态
    updateNetworkState();
    
    return true;
}

bool SimulationEngine::step()
{
    if (m_Running && m_ShouldRun) {
        std::cerr << "无法在自动模拟运行时执行单步" << std::endl;
        return false;
    }
    
    // 确保网络已加载
    if (!m_Network) {
        std::cerr << "无法执行单步模拟：网络未加载" << std::endl;
        return false;
    }
    
    // 执行一个Tick-Tock周期
    std::cout << "== 开始单步模拟，当前tick: " << m_CurrentTick << " ==" << std::endl;
    
    // 执行Tick
    std::cout << "Tick 阶段开始..." << std::endl;
    m_Network->tick();
    std::cout << "Tick 阶段完成" << std::endl;
    
    // 执行Tock
    std::cout << "Tock 阶段开始..." << std::endl;
    m_Network->tock();
    std::cout << "Tock 阶段完成" << std::endl;
    
    // 更新计数器和状态
    m_CurrentTick = m_Network->getTickTock();
    updateNetworkState();
    
    std::cout << "== 单步模拟完成，当前tick: " << m_CurrentTick << " ==" << std::endl;
    
    return true;
}

bool SimulationEngine::reset()
{
    // 如果正在运行，需要先停止
    if (m_Running) {
        stop();
    }
    
    // 确保网络已加载
    if (!m_Network) {
        std::cerr << "无法重置模拟：网络未加载" << std::endl;
        return false;
    }
    
    // 重新创建网络
    try {
        // 保存当前网络的序列化表示
        std::string networkJson = m_Network->serialize();
        
        // 重新创建网络
        m_Network = NetworkFactory::deserializeNetwork(networkJson);
        
        // 重置状态
        m_CurrentTick = 0;
        updateNetworkState();
        
        std::cout << "模拟已重置" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "重置模拟失败: " << e.what() << std::endl;
        return false;
    }
}

json SimulationEngine::getState() const
{
    std::lock_guard<std::mutex> lock(m_StateMutex);
    return m_NetworkState;
}

bool SimulationEngine::isRunning() const
{
    return m_Running;
}

int SimulationEngine::getCurrentTick() const
{
    return m_CurrentTick;
}

void SimulationEngine::simulationThread()
{
    std::cout << "模拟线程已启动" << std::endl;
    
    while (m_ShouldRun)
    {
        // 执行一个Tick-Tock周期
        std::cout << "== 自动模拟，当前tick: " << m_CurrentTick << " ==" << std::endl;
        
        // 执行Tick
        std::cout << "Tick 阶段开始..." << std::endl;
        m_Network->tick();
        std::cout << "Tick 阶段完成" << std::endl;
        
        // 执行Tock
        std::cout << "Tock 阶段开始..." << std::endl;
        m_Network->tock();
        std::cout << "Tock 阶段完成" << std::endl;
        
        // 更新计数器和状态
        m_CurrentTick = m_Network->getTickTock();
        updateNetworkState();
        
        std::cout << "== 自动模拟完成，当前tick: " << m_CurrentTick << " ==" << std::endl;
        
        // 模拟速度控制
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    std::cout << "模拟线程已退出" << std::endl;
}

void SimulationEngine::updateNetworkState()
{
    // 确保网络已加载
    if (!m_Network) {
        m_NetworkState = json("{\"tick\":0,\"running\":false,\"nodes\":[],\"connections\":[]}");
        return;
    }
    
    // 获取网络状态的JSON表示
    try {
        std::string networkJson = m_Network->serialize();
        
        // 解析为json对象
        auto j = nlohmann::json::parse(networkJson);
        
        // 添加运行状态信息
        j["running"] = m_Running ? true : false;
        
        // 更新状态
        m_NetworkState = json(j.dump());
    }
    catch (const std::exception& e) {
        std::cerr << "更新网络状态失败: " << e.what() << std::endl;
        m_NetworkState = json("{\"tick\":" + std::to_string(m_CurrentTick) + 
                               ",\"running\":" + (m_Running ? "true" : "false") + 
                               ",\"error\":\"" + e.what() + "\"}");
    }
}

} // namespace sim 