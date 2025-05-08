#include "server.h"
#include "network_factory.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <random>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <httplib.h>

namespace sim {

Server::Server(int port)
    : m_Port(port)
    , m_Running(false)
    , m_NetworkState(json("{\"tick\":0,\"running\":false,\"nodes\":[],\"connections\":[]}"))
    , m_SimEngine(std::make_unique<SimulationEngine>())
    , m_HttpServer(std::make_unique<httplib::Server>())
{
}

Server::~Server()
{
    stop();
}

void Server::initHttpRoutes()
{
    // 健康检查接口
    m_HttpServer->Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    // 获取网络状态
    m_HttpServer->Get("/api/network/state", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(m_StateMutex);
        res.set_content(m_NetworkState.dump(), "application/json");
    });

    // 加载网络配置
    m_HttpServer->Post("/api/network/load", [this](const httplib::Request& req, httplib::Response& res) {
        bool success = loadNetworkFromJson(req.body);
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"加载网络失败\"}", "application/json");
        }
    });

    // 启动模拟
    m_HttpServer->Post("/api/simulation/start", [this](const httplib::Request&, httplib::Response& res) {
        bool success = startSimulation();
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"启动模拟失败\"}", "application/json");
        }
    });

    // 停止模拟
    m_HttpServer->Post("/api/simulation/stop", [this](const httplib::Request&, httplib::Response& res) {
        bool success = stopSimulation();
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"停止模拟失败\"}", "application/json");
        }
    });

    // 单步执行
    m_HttpServer->Post("/api/simulation/step", [this](const httplib::Request&, httplib::Response& res) {
        bool success = stepSimulation();
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"单步执行失败\"}", "application/json");
        }
    });

    // 重置模拟
    m_HttpServer->Post("/api/simulation/reset", [this](const httplib::Request&, httplib::Response& res) {
        bool success = resetSimulation();
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"重置模拟失败\"}", "application/json");
        }
    });

    // 创建生产者-消费者网络
    m_HttpServer->Post("/api/network/create/producer-consumer", [this](const httplib::Request&, httplib::Response& res) {
        bool success = createProducerConsumerNetwork();
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"创建网络失败\"}", "application/json");
        }
    });

    // 安全停止服务器
    m_HttpServer->Post("/api/server/shutdown", [this](const httplib::Request&, httplib::Response& res) {
        bool success = shutdown();
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"停止服务器失败\"}", "application/json");
        }
    });
}

bool Server::start()
{
    if (m_Running) {
        return true;
    }
    
    // 初始化HTTP路由
    initHttpRoutes();
    
    // 启动HTTP服务器
    m_Running = true;
    m_ServerThread = std::thread([this]() {
        std::cout << "HTTP服务器已启动，监听端口: " << m_Port << std::endl;
        m_HttpServer->listen("0.0.0.0", m_Port);
    });
    
    return true;
}

void Server::stop()
{
    if (!m_Running) {
        return;
    }
    
    // 先停止模拟
    stopSimulation();
    
    // 停止HTTP服务器
    m_Running = false;
    m_HttpServer->stop();
    
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
        
        // 等待状态更新完成
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 再次更新状态以确保同步
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

void Server::updateNetworkState()
{
    // 从模拟引擎获取最新状态
    json newState = m_SimEngine->getState();
    
    // 检查状态是否变化
    {
        std::lock_guard<std::mutex> lock(m_StateMutex);
        m_NetworkState = newState;
        
        // 广播状态更新
        broadcastStateUpdate(m_NetworkState);
    }
}

bool Server::shutdown()
{
    if (!m_Running) {
        return true;
    }
    
    // 先停止模拟
    stopSimulation();
    
    // 设置停止标志
    m_Running = false;
    
    // 在后台线程中停止服务器
    std::thread([this]() {
        // 等待一小段时间确保响应已经发送
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 停止HTTP服务器
        m_HttpServer->stop();
        
        if (m_ServerThread.joinable()) {
            m_ServerThread.join();
        }
        
        std::cout << "服务器已安全停止" << std::endl;
    }).detach();
    
    return true;
}

// SimulationEngine 类的实现保持不变
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