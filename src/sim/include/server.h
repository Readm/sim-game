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
 * @brief Network simulation server
 * 
 * Provides HTTP interface for controlling network simulation and getting status updates
 */
class Server {
public:
    /**
     * @brief Construct server
     * @param port Server port number
     */
    Server(int port = 8080);
    
    /**
     * @brief Destructor
     */
    virtual ~Server();
    
    /**
     * @brief Start server
     * @return Whether started successfully
     */
    bool start();
    
    /**
     * @brief Stop server
     */
    void stop();
    
    /**
     * @brief Check if server is running
     * @return Running status
     */
    bool isRunning() const;
    
    /**
     * @brief Load network configuration from file
     * @param filepath JSON file path
     * @return Whether loaded successfully
     */
    bool loadNetworkFromFile(const std::string& filepath);
    
    /**
     * @brief Load network configuration from JSON string
     * @param jsonStr JSON string
     * @return Whether loaded successfully
     */
    bool loadNetworkFromJson(const std::string& jsonStr);
    
    /**
     * @brief Get current network state
     * @return Network state JSON
     */
    json getNetworkState() const;
    
    /**
     * @brief Start simulation
     * @return Whether started successfully
     */
    bool startSimulation();
    
    /**
     * @brief Stop simulation
     * @return Whether stopped successfully
     */
    bool stopSimulation();
    
    /**
     * @brief Execute simulation step
     * @return Whether executed successfully
     */
    bool stepSimulation();
    
    /**
     * @brief Reset simulation
     * @return Whether reset successfully
     */
    bool resetSimulation();
    
    /**
     * @brief Safely stop server
     * @return Whether stopped successfully
     */
    bool shutdown();
    
    /**
     * @brief Set state update callback
     * @param callback Callback function
     */
    void setStateUpdateCallback(std::function<void(const json&)> callback);
    
    /**
     * @brief Broadcast state update
     * @param state State JSON
     */
    void broadcastStateUpdate(const json& state);
    
    /**
     * @brief Get request history (for testing)
     * @return Request log
     */
    std::vector<std::string> getRequestLog() const;
    
    /**
     * @brief Clear request history
     */
    void clearRequestLog();
    
    /**
     * @brief Create producer-consumer network
     * @return Whether created successfully
     */
    bool createProducerConsumerNetwork();

private:
    // Initialize HTTP routes
    void initHttpRoutes();
    
    // Update network state
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
 * @brief Simulation engine class
 * 
 * Responsible for actual network simulation calculation, server only provides API interface
 */
class SimulationEngine {
public:
    SimulationEngine();
    ~SimulationEngine();
    
    // Load network
    bool loadFromFile(const std::string& filepath);
    bool loadFromJson(const std::string& jsonStr);
    
    // Create predefined network
    bool createProducerConsumerNetwork();
    
    // Control simulation
    bool start();
    bool stop();
    bool step();
    bool reset();
    
    // Get state
    json getState() const;
    bool isRunning() const;
    int getCurrentTick() const;

private:
    // Simulation thread function
    void simulationThread();
    
    // Update network state
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