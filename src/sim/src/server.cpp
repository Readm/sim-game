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
    // Health check endpoint
    m_HttpServer->Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    // Get network state
    m_HttpServer->Get("/api/network/state", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(m_StateMutex);
        res.set_content(m_NetworkState.dump(), "application/json");
    });

    // Load network configuration
    m_HttpServer->Post("/api/network/load", [this](const httplib::Request& req, httplib::Response& res) {
        bool success = loadNetworkFromJson(req.body);
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"Failed to load network\"}", "application/json");
        }
    });

    // Start simulation
    m_HttpServer->Post("/api/simulation/start", [this](const httplib::Request&, httplib::Response& res) {
        bool success = startSimulation();
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"Failed to start simulation\"}", "application/json");
        }
    });

    // Stop simulation
    m_HttpServer->Post("/api/simulation/stop", [this](const httplib::Request&, httplib::Response& res) {
        bool success = stopSimulation();
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"Failed to stop simulation\"}", "application/json");
        }
    });

    // Step simulation
    m_HttpServer->Post("/api/simulation/step", [this](const httplib::Request&, httplib::Response& res) {
        bool success = stepSimulation();
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"Failed to step simulation\"}", "application/json");
        }
    });

    // Reset simulation
    m_HttpServer->Post("/api/simulation/reset", [this](const httplib::Request&, httplib::Response& res) {
        bool success = resetSimulation();
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"Failed to reset simulation\"}", "application/json");
        }
    });

    // Create producer-consumer network
    m_HttpServer->Post("/api/network/create/producer-consumer", [this](const httplib::Request&, httplib::Response& res) {
        bool success = createProducerConsumerNetwork();
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"Failed to create network\"}", "application/json");
        }
    });

    // Safely shutdown server
    m_HttpServer->Post("/api/server/shutdown", [this](const httplib::Request&, httplib::Response& res) {
        bool success = shutdown();
        if (success) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"Failed to stop server\"}", "application/json");
        }
    });
}

bool Server::start()
{
    if (m_Running) {
        return true;
    }
    
    // Initialize HTTP routes
    initHttpRoutes();
    
    // Start HTTP server
    m_Running = true;
    m_ServerThread = std::thread([this]() {
        std::cout << "HTTP server started, listening on port: " << m_Port << std::endl;
        m_HttpServer->listen("0.0.0.0", m_Port);
    });
    
    return true;
}

void Server::stop()
{
    if (!m_Running) {
        return;
    }
    
    // Stop simulation first
    stopSimulation();
    
    // Stop HTTP server
    m_Running = false;
    m_HttpServer->stop();
    
    if (m_ServerThread.joinable()) {
        m_ServerThread.join();
    }
    
    std::cout << "Server stopped" << std::endl;
}

bool Server::isRunning() const
{
    return m_Running;
}

bool Server::loadNetworkFromFile(const std::string& filepath)
{
    // Check if file exists
    if (!std::filesystem::exists(filepath)) {
        std::cerr << "File does not exist: " << filepath << std::endl;
        return false;
    }
    
    // Read file content
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filepath << std::endl;
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
        
        std::cout << "Server safely stopped" << std::endl;
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
    // Check if file exists
    if (!std::filesystem::exists(filepath)) {
        std::cerr << "File does not exist: " << filepath << std::endl;
        return false;
    }
    
    // Read file content
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return loadFromJson(buffer.str());
}

bool SimulationEngine::loadFromJson(const std::string& jsonStr)
{
    // Check if simulation is currently running
    if (m_Running) {
        std::cerr << "Cannot load network while simulation is running" << std::endl;
        return false;
    }
    
    try {
        // Deserialize network
        m_Network = NetworkFactory::deserializeNetwork(jsonStr);
        
        // Reset state
        m_CurrentTick = 0;
        
        {
            std::lock_guard<std::mutex> lock(m_StateMutex);
            // Update network state
            updateNetworkState();
        }
        
        std::cout << "Network configuration loaded successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load network: " << e.what() << std::endl;
        return false;
    }
}

bool SimulationEngine::createProducerConsumerNetwork()
{
    // Check if simulation is currently running
    if (m_Running) {
        std::cerr << "Cannot create network while simulation is running" << std::endl;
        return false;
    }
    
    try {
        // Create producer-consumer network
        m_Network = NetworkFactory::createProducerConsumerNetwork();
        
        // Reset state
        m_CurrentTick = 0;
        
        {
            std::lock_guard<std::mutex> lock(m_StateMutex);
            // Update network state
            updateNetworkState();
        }
        
        std::cout << "Producer-consumer network created successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to create network: " << e.what() << std::endl;
        return false;
    }
}

bool SimulationEngine::start()
{
    if (m_Running) {
        return true; // Already running
    }
    
    // Ensure network is loaded
    if (!m_Network) {
        std::cerr << "Cannot start simulation: Network not loaded" << std::endl;
        return false;
    }
    
    m_ShouldRun = true;
    m_Running = true;
    m_SimThread = std::thread(&SimulationEngine::simulationThread, this);
    
    std::cout << "Simulation started" << std::endl;
    return true;
}

bool SimulationEngine::stop()
{
    if (!m_Running) {
        return true; // Already stopped
    }
    
    m_ShouldRun = false;
    
    if (m_SimThread.joinable()) {
        m_SimThread.join();
    }
    
    m_Running = false;
    
    std::cout << "Simulation stopped" << std::endl;
    
    // Update state
    updateNetworkState();
    
    return true;
}

bool SimulationEngine::step()
{
    if (m_Running && m_ShouldRun) {
        std::cerr << "Cannot step while automatic simulation is running" << std::endl;
        return false;
    }
    
    // Ensure network is loaded
    if (!m_Network) {
        std::cerr << "Cannot step simulation: Network not loaded" << std::endl;
        return false;
    }
    
    // Execute a Tick-Tock cycle
    std::cout << "== Starting step simulation, current tick: " << m_CurrentTick << " ==" << std::endl;
    
    // Execute Tick
    std::cout << "Tick phase starting..." << std::endl;
    m_Network->tick();
    std::cout << "Tick phase completed" << std::endl;
    
    // Execute Tock
    std::cout << "Tock phase starting..." << std::endl;
    m_Network->tock();
    std::cout << "Tock phase completed" << std::endl;
    
    // Update counter and state
    m_CurrentTick = m_Network->getTickTock();
    updateNetworkState();
    
    std::cout << "== Step simulation completed, current tick: " << m_CurrentTick << " ==" << std::endl;
    
    return true;
}

bool SimulationEngine::reset()
{
    // If running, stop first
    if (m_Running) {
        stop();
    }
    
    // Ensure network is loaded
    if (!m_Network) {
        std::cerr << "Cannot reset simulation: Network not loaded" << std::endl;
        return false;
    }
    
    // Recreate network
    try {
        // Save current network serialization
        std::string networkJson = m_Network->serialize();
        
        // Recreate network
        m_Network = NetworkFactory::deserializeNetwork(networkJson);
        
        // Reset state
        m_CurrentTick = 0;
        updateNetworkState();
        
        std::cout << "Simulation reset" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to reset simulation: " << e.what() << std::endl;
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
    std::cout << "Simulation thread started" << std::endl;
    
    while (m_ShouldRun)
    {
        // Execute a Tick-Tock cycle
        std::cout << "== Auto simulation, current tick: " << m_CurrentTick << " ==" << std::endl;
        
        // Execute Tick
        std::cout << "Tick phase starting..." << std::endl;
        m_Network->tick();
        std::cout << "Tick phase completed" << std::endl;
        
        // Execute Tock
        std::cout << "Tock phase starting..." << std::endl;
        m_Network->tock();
        std::cout << "Tock phase completed" << std::endl;
        
        // Update counter and state
        m_CurrentTick = m_Network->getTickTock();
        updateNetworkState();
        
        std::cout << "== Auto simulation completed, current tick: " << m_CurrentTick << " ==" << std::endl;
        
        // Simulation speed control
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    std::cout << "Simulation thread exited" << std::endl;
}

void SimulationEngine::updateNetworkState()
{
    // Ensure network is loaded
    if (!m_Network) {
        m_NetworkState = json("{\"tick\":0,\"running\":false,\"nodes\":[],\"connections\":[]}");
        return;
    }
    
    // Get network state JSON representation
    try {
        std::string networkJson = m_Network->serialize();
        
        // Parse as json object
        auto j = nlohmann::json::parse(networkJson);
        
        // Add running state information
        j["running"] = m_Running ? true : false;
        
        // Update state
        m_NetworkState = json(j.dump());
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to update network state: " << e.what() << std::endl;
        m_NetworkState = json("{\"tick\":" + std::to_string(m_CurrentTick) + 
                               ",\"running\":" + (m_Running ? "true" : "false") + 
                               ",\"error\":\"" + e.what() + "\"}");
    }
}

} // namespace sim 