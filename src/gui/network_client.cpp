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
    std::cout << "正在连接到服务器: " << m_BaseUrl << std::endl;
    
    // 尝试连接次数
    const int maxRetries = 5;
    for (int i = 0; i < maxRetries; i++) {
        try {
            // 模拟连接延迟 - 实际上是给服务器一些响应时间
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            
            // 尝试发送一个简单的Get请求验证服务器可用性
            json response = get("/api/health");
            
            // 如果能够成功获取响应，则认为已连接
            m_Connected = true;
            std::cout << "已连接到服务器: " << m_BaseUrl << " (尝试 " << (i+1) << "/" << maxRetries << ")" << std::endl;
            
            // 获取初始网络状态
            json initialState = getNetworkState();
            std::cout << "初始网络状态: " << initialState.dump().substr(0, 100) << "..." << std::endl;
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "连接尝试 " << (i+1) << "/" << maxRetries << " 失败: " << e.what() << std::endl;
            
            // 最后一次尝试失败
            if (i == maxRetries - 1) {
                m_Connected = false;
                std::cerr << "连接到服务器失败，已达最大重试次数" << std::endl;
                return false;
            }
            
            // 等待时间逐次增加
            std::this_thread::sleep_for(std::chrono::milliseconds(300 * (i + 1)));
        }
    }
    
    // 所有尝试都失败
    m_Connected = false;
    std::cerr << "连接到服务器失败: " << m_BaseUrl << std::endl;
    return false;
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
        
    // 发送启动请求
    json response = post("/api/simulation/start", json());
    
    // 模拟状态变化并通知观察者
    if (m_StateUpdateCallback) {
        // 构建模拟"运行中"状态
        json runningState = getNetworkState();
        
        // 修改状态为运行中
        try {
            if (runningState.dump() != "{}") {
                runningState["data"]["running"] = true;
                
                // 调用回调通知观察者
                m_StateUpdateCallback(runningState);
            }
        } catch (const std::exception& e) {
            std::cerr << "更新运行状态时出错: " << e.what() << std::endl;
        }
    }
    
    std::cout << "模拟已启动" << std::endl;
    return true;
}

bool NetworkClient::stopSimulation()
{
    if (!m_Connected)
        return false;
        
    // 发送停止请求
    json response = post("/api/simulation/stop", json());
    
    // 模拟状态变化并通知观察者
    if (m_StateUpdateCallback) {
        // 构建模拟"已停止"状态
        json stoppedState = getNetworkState();
        
        // 修改状态为已停止
        try {
            if (stoppedState.dump() != "{}") {
                stoppedState["data"]["running"] = false;
                
                // 调用回调通知观察者
                m_StateUpdateCallback(stoppedState);
            }
        } catch (const std::exception& e) {
            std::cerr << "更新停止状态时出错: " << e.what() << std::endl;
        }
    }
    
    std::cout << "模拟已停止" << std::endl;
    return true;
}

bool NetworkClient::stepSimulation()
{
    if (!m_Connected)
        return false;
        
    static int stepCount = 0;
    stepCount++;
    
    // 发送步进请求
    json response = post("/api/simulation/step", json());
    
    // 模拟状态变化并通知观察者
    if (m_StateUpdateCallback) {
        // 构建更新后的状态
        json updatedState = json(R"({
            "status": "success",
            "data": {
                "tick": 0,
                "running": false,
                "nodes": [
                    {
                        "id": 1,
                        "type": "producer",
                        "name": "生产者节点",
                        "properties": {
                            "produced_count": 0
                        },
                        "position": {
                            "x": 100,
                            "y": 100
                        },
                        "inputs": [],
                        "outputs": [
                            {
                                "id": 1,
                                "name": "out",
                                "type": "void",
                                "connected": true
                            }
                        ]
                    },
                    {
                        "id": 2,
                        "type": "consumer",
                        "name": "消费者节点",
                        "properties": {
                            "consumed_count": 0
                        },
                        "position": {
                            "x": 400,
                            "y": 100
                        },
                        "inputs": [
                            {
                                "id": 2,
                                "name": "in",
                                "type": "void",
                                "connected": true
                            }
                        ],
                        "outputs": []
                    }
                ],
                "connections": [
                    {
                        "id": 1,
                        "from_node": 1,
                        "from_port": 1,
                        "to_node": 2,
                        "to_port": 2
                    }
                ]
            }
        })");
        
        // 更新tick和计数器
        updatedState["data"]["tick"] = stepCount;
        updatedState["data"]["nodes"][0]["properties"]["produced_count"] = stepCount;
        updatedState["data"]["nodes"][1]["properties"]["consumed_count"] = stepCount;
        
        // 调用回调通知观察者
        m_StateUpdateCallback(updatedState);
    }
    
    std::cout << "模拟已步进 (Step " << stepCount << ")" << std::endl;
    return true;
}

bool NetworkClient::resetSimulation()
{
    if (!m_Connected)
        return false;
        
    // 重置步进计数
    static int* stepCount = new int(0);
    *stepCount = 0;
    
    // 发送重置请求
    json response = post("/api/simulation/reset", json());
    
    // 模拟状态变化并通知观察者
    if (m_StateUpdateCallback) {
        // 构建重置后的状态
        json resetState = json(R"({
            "status": "success",
            "data": {
                "tick": 0,
                "running": false,
                "nodes": [
                    {
                        "id": 1,
                        "type": "producer",
                        "name": "生产者节点",
                        "properties": {
                            "produced_count": 0
                        },
                        "position": {
                            "x": 100,
                            "y": 100
                        },
                        "inputs": [],
                        "outputs": [
                            {
                                "id": 1,
                                "name": "out",
                                "type": "void",
                                "connected": true
                            }
                        ]
                    },
                    {
                        "id": 2,
                        "type": "consumer",
                        "name": "消费者节点",
                        "properties": {
                            "consumed_count": 0
                        },
                        "position": {
                            "x": 400,
                            "y": 100
                        },
                        "inputs": [
                            {
                                "id": 2,
                                "name": "in",
                                "type": "void",
                                "connected": true
                            }
                        ],
                        "outputs": []
                    }
                ],
                "connections": [
                    {
                        "id": 1,
                        "from_node": 1,
                        "from_port": 1,
                        "to_node": 2,
                        "to_port": 2
                    }
                ]
            }
        })");
        
        // 调用回调通知观察者
        m_StateUpdateCallback(resetState);
    }
    
    std::cout << "模拟已重置" << std::endl;
    return true;
}

json NetworkClient::getNetworkState()
{
    if (!m_Connected)
        return json("{}");
    
    try {
        // 尝试获取当前网络状态
        json networkState = get("/api/network/state");
        return networkState;
    } catch (const std::exception& e) {
        std::cerr << "获取网络状态失败: " << e.what() << std::endl;
        return json("{}");
    }
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
    
    try {
        // 模拟网络延迟
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        // 这里是模拟返回
        if (endpoint == "/api/health") {
            return json("{\"status\":\"ok\"}");
        } else if (endpoint == "/api/network/state") {
            // 返回一个完整的网络状态
            return json(R"({
                "status": "success",
                "data": {
                    "tick": 0,
                    "running": false,
                    "nodes": [
                        {
                            "id": 1,
                            "type": "producer",
                            "name": "生产者节点",
                            "properties": {
                                "produced_count": 0
                            },
                            "position": {
                                "x": 100,
                                "y": 100
                            },
                            "inputs": [],
                            "outputs": [
                                {
                                    "id": 1,
                                    "name": "out",
                                    "type": "void",
                                    "connected": true
                                }
                            ]
                        },
                        {
                            "id": 2,
                            "type": "consumer",
                            "name": "消费者节点",
                            "properties": {
                                "consumed_count": 0
                            },
                            "position": {
                                "x": 400,
                                "y": 100
                            },
                            "inputs": [
                                {
                                    "id": 2,
                                    "name": "in",
                                    "type": "void",
                                    "connected": true
                                }
                            ],
                            "outputs": []
                        }
                    ],
                    "connections": [
                        {
                            "id": 1,
                            "from_node": 1,
                            "from_port": 1,
                            "to_node": 2,
                            "to_port": 2
                        }
                    ]
                }
            })");
        }
        
        // 默认返回
        return json("{\"status\":\"success\"}");
    } catch (const std::exception& e) {
        std::cerr << "GET请求失败: " << e.what() << std::endl;
        throw; // 重新抛出异常
    }
}

json NetworkClient::post(const std::string& endpoint, const json& data)
{
    std::cout << "POST " << m_BaseUrl << endpoint << " 数据: " << data.dump() << std::endl;
    
    try {
        // 模拟网络延迟
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        // 处理不同的端点
        if (endpoint == "/api/simulation/start") {
            // 模拟模拟启动成功
            return json("{\"status\":\"success\",\"message\":\"模拟已启动\"}");
        } else if (endpoint == "/api/simulation/stop") {
            // 模拟模拟停止成功
            return json("{\"status\":\"success\",\"message\":\"模拟已停止\"}");
        } else if (endpoint == "/api/simulation/step") {
            // 模拟单步执行成功
            static int tick = 0;
            tick++;
            return json("{\"status\":\"success\",\"message\":\"模拟已步进\",\"data\":{\"tick\":" + std::to_string(tick) + "}}");
        } else if (endpoint == "/api/simulation/reset") {
            // 模拟重置成功
            static int tick = 0;
            tick = 0;
            return json("{\"status\":\"success\",\"message\":\"模拟已重置\",\"data\":{\"tick\":0}}");
        }
        
        // 默认返回
        return json("{\"status\":\"success\"}");
    } catch (const std::exception& e) {
        std::cerr << "POST请求失败: " << e.what() << std::endl;
        return json("{\"status\":\"error\",\"message\":\"" + std::string(e.what()) + "\"}");
    }
}

json NetworkClient::put(const std::string& endpoint, const json& data)
{
    std::cout << "PUT " << m_BaseUrl << endpoint << " 数据: " << data.dump() << std::endl;
    
    try {
        // 模拟网络延迟
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        // 处理节点更新请求
        if (endpoint.find("/api/network/nodes/") != std::string::npos) {
            int nodeId = std::stoi(endpoint.substr(endpoint.find_last_of('/') + 1));
            return json("{\"status\":\"success\",\"message\":\"节点 " + std::to_string(nodeId) + " 已更新\"}");
        }
        
        // 默认返回
        return json("{\"status\":\"success\"}");
    } catch (const std::exception& e) {
        std::cerr << "PUT请求失败: " << e.what() << std::endl;
        return json("{\"status\":\"error\",\"message\":\"" + std::string(e.what()) + "\"}");
    }
}

json NetworkClient::del(const std::string& endpoint)
{
    std::cout << "DELETE " << m_BaseUrl << endpoint << std::endl;
    
    try {
        // 模拟网络延迟
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        // 处理连接删除请求
        if (endpoint.find("/api/network/connections/") != std::string::npos) {
            int connectionId = std::stoi(endpoint.substr(endpoint.find_last_of('/') + 1));
            return json("{\"status\":\"success\",\"message\":\"连接 " + std::to_string(connectionId) + " 已删除\"}");
        }
        
        // 默认返回
        return json("{\"status\":\"success\"}");
    } catch (const std::exception& e) {
        std::cerr << "DELETE请求失败: " << e.what() << std::endl;
        return json("{\"status\":\"error\",\"message\":\"" + std::string(e.what()) + "\"}");
    }
} 