# API 参考文档

## 核心类接口

### Node 类

#### 基础接口
```cpp
class Node {
public:
    // 构造和析构
    Node(NodeID id, const std::string& name);
    virtual ~Node();
    
    // 核心模拟接口
    virtual void Tick(TickTock current_time) = 0;
    virtual void Tock(TickTock current_time) = 0;
    
    // 序列化接口
    virtual nlohmann::json serialize() const = 0;
    virtual void deserialize(const nlohmann::json& json) = 0;
    
    // 子节点管理
    void AddChildNode(std::shared_ptr<Node> child);
    void RemoveChildNode(NodeID child_id);
    std::vector<std::shared_ptr<Node>> GetChildNodes() const;
    
    // 端口管理
    void AddInputPort(const std::string& name, size_t capacity = 0);
    void AddOutputPort(const std::string& name);
    InputPort* GetInputPort(const std::string& name);
    OutputPort* GetOutputPort(const std::string& name);
    
    // 状态查询
    NodeID GetID() const;
    std::string GetName() const;
    TickTock GetCurrentTime() const;
    
    // 显示状态管理
    DisplayState& GetDisplayState();
    void SetDisplayState(const DisplayState& state);
};
```

#### 使用示例
```cpp
// 创建自定义节点
class MyNode : public Node {
public:
    MyNode(NodeID id) : Node(id, "MyNode") {
        AddInputPort("input", 10);  // 容量为10的输入端口
        AddOutputPort("output");    // 输出端口
    }
    
    void Tick(TickTock current_time) override {
        // 读取输入端口状态
        auto input_port = GetInputPort("input");
        if (input_port && !input_port->IsEmpty()) {
            // 处理输入数据
        }
    }
    
    void Tock(TickTock current_time) override {
        // 更新节点状态，生成输出
        auto output_port = GetOutputPort("output");
        if (output_port) {
            auto packet = Spawn<VoidPacket>();
            output_port->Send(packet);
        }
    }
    
    nlohmann::json serialize() const override {
        auto json = Node::serialize();
        // 添加自定义序列化内容
        return json;
    }
    
    void deserialize(const nlohmann::json& json) override {
        Node::deserialize(json);
        // 添加自定义反序列化内容
    }
};
```

### Packet 类

#### 基础接口
```cpp
class Packet {
public:
    // 构造函数（由Node调用）
    Packet(NodeID src_node_id, PacketID id);
    virtual ~Packet();
    
    // 序列化接口
    virtual nlohmann::json serialize() const = 0;
    virtual void deserialize(const nlohmann::json& json) = 0;
    
    // 基础属性
    NodeID GetSourceNodeID() const;
    PacketID GetID() const;
    TickTock GetTimestamp() const;
    
    // 载荷管理
    void AddPayload(const std::string& key, PacketID payload_id);
    PacketID GetPayload(const std::string& key) const;
    bool HasPayload(const std::string& key) const;
    
    // 静态方法
    static std::shared_ptr<Packet> GetPacketByID(PacketID id);
};
```

#### 使用示例
```cpp
// 创建自定义数据包
class DataPacket : public Packet {
private:
    int data_value;
    std::string message;
    
public:
    DataPacket(NodeID src_id, PacketID id, int value, const std::string& msg)
        : Packet(src_id, id), data_value(value), message(msg) {}
    
    int GetDataValue() const { return data_value; }
    std::string GetMessage() const { return message; }
    
    nlohmann::json serialize() const override {
        auto json = Packet::serialize();
        json["data_value"] = data_value;
        json["message"] = message;
        return json;
    }
    
    void deserialize(const nlohmann::json& json) override {
        Packet::deserialize(json);
        data_value = json["data_value"];
        message = json["message"];
    }
};

// 在Node中生成数据包
auto packet = Spawn<DataPacket>(42, "Hello World");
```

### Port 类

#### InputPort 接口
```cpp
class InputPort {
public:
    InputPort(const std::string& name, size_t capacity = 0);
    
    // 数据接收
    bool Receive(std::shared_ptr<Packet> packet);
    std::shared_ptr<Packet> Pop();
    std::shared_ptr<Packet> Peek() const;
    
    // 状态查询
    bool IsEmpty() const;
    bool IsFull() const;
    size_t GetSize() const;
    size_t GetCapacity() const;
    std::string GetName() const;
    
    // 连接管理
    void ConnectFrom(OutputPort* output_port);
    void Disconnect();
    bool IsConnected() const;
};
```

#### OutputPort 接口
```cpp
class OutputPort {
public:
    OutputPort(const std::string& name);
    
    // 数据发送
    bool Send(std::shared_ptr<Packet> packet);
    bool CanSend() const;
    
    // 连接管理
    void ConnectTo(InputPort* input_port);
    void Disconnect(InputPort* input_port);
    void DisconnectAll();
    
    // 状态查询
    std::vector<InputPort*> GetConnectedPorts() const;
    std::string GetName() const;
    size_t GetConnectionCount() const;
};
```

## 网络和工厂类

### Network 类
```cpp
class Network : public Node {
public:
    Network();
    
    // 网络控制
    void Start();
    void Stop();
    void Step();
    void Reset();
    
    // 状态查询
    bool IsRunning() const;
    TickTock GetCurrentTick() const;
    
    // 统计信息
    size_t GetTotalNodes() const;
    size_t GetTotalPackets() const;
    double GetSimulationSpeed() const;
};
```

### NetworkFactory 类
```cpp
class NetworkFactory {
public:
    // 预定义网络创建
    static std::shared_ptr<Network> CreateSimpleNetwork();
    static std::shared_ptr<Network> CreateProducerConsumerNetwork();
    static std::shared_ptr<Network> CreatePipelineNetwork(size_t stages);
    
    // 从配置创建
    static std::shared_ptr<Network> CreateFromJSON(const nlohmann::json& config);
    static std::shared_ptr<Network> LoadFromFile(const std::string& filename);
    
    // 网络保存
    static bool SaveToFile(std::shared_ptr<Network> network, const std::string& filename);
    static nlohmann::json NetworkToJSON(std::shared_ptr<Network> network);
};
```

## 服务器API

### HTTP API 接口

#### 网络控制
```http
POST /api/network/start
POST /api/network/stop
POST /api/network/step
POST /api/network/reset

GET /api/network/status
Response: {
    "running": true,
    "current_tick": 1234,
    "total_nodes": 10,
    "total_packets": 56
}
```

#### 网络配置
```http
GET /api/network/config
POST /api/network/config
PUT /api/network/config

GET /api/network/nodes
GET /api/network/nodes/{node_id}
POST /api/network/nodes
DELETE /api/network/nodes/{node_id}
```

#### 数据查询
```http
GET /api/network/packets
GET /api/network/packets/{packet_id}
GET /api/network/statistics
```

### WebSocket API

#### 连接和认证
```javascript
// 连接WebSocket
const ws = new WebSocket('ws://localhost:8080/ws');

// 发送认证消息
ws.send(JSON.stringify({
    type: 'auth',
    token: 'your_token_here'
}));
```

#### 实时状态更新
```javascript
// 订阅状态更新
ws.send(JSON.stringify({
    type: 'subscribe',
    events: ['node_update', 'packet_update', 'network_status']
}));

// 接收状态更新
ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    switch(data.type) {
        case 'node_update':
            // 处理节点状态更新
            break;
        case 'packet_update':
            // 处理数据包更新
            break;
        case 'network_status':
            // 处理网络状态更新
            break;
    }
};
```

## 类型系统

### TypeID 系统
```cpp
// 类型注册
template<typename T>
class TypeRegistry {
public:
    static TypeID GetTypeID() {
        static TypeID id = Hash(typeid(T).name());
        return id;
    }
    
    static void RegisterType() {
        auto id = GetTypeID();
        // 检查ID冲突
        if (registered_types.count(id)) {
            throw std::runtime_error("TypeID collision detected");
        }
        registered_types.insert(id);
    }
};

// 使用示例
class MyNode : public Node {
public:
    static TypeID GetStaticTypeID() {
        return TypeRegistry<MyNode>::GetTypeID();
    }
    
    TypeID GetTypeID() const override {
        return GetStaticTypeID();
    }
};

// 注册类型（在程序启动时）
TypeRegistry<MyNode>::RegisterType();
```

### 序列化格式

#### JSON 格式
```json
{
    "type_id": 12345,
    "node_id": 67890,
    "name": "MyNode",
    "current_time": 1000,
    "display_state": {
        "expanded": false,
        "position": {"x": 100, "y": 200},
        "view_level": 0
    },
    "input_ports": [
        {
            "name": "input",
            "capacity": 10,
            "packets": []
        }
    ],
    "output_ports": [
        {
            "name": "output",
            "connections": [{"node_id": 111, "port_name": "input"}]
        }
    ],
    "child_nodes": [],
    "buffer": []
}
```

## 错误处理

### 异常类型
```cpp
// 基础异常类
class SimulationException : public std::exception {
public:
    SimulationException(const std::string& message);
    const char* what() const noexcept override;
};

// 具体异常类型
class NodeNotFoundException : public SimulationException {};
class PacketNotFoundException : public SimulationException {};
class PortConnectionException : public SimulationException {};
class SerializationException : public SimulationException {};
class TypeIDCollisionException : public SimulationException {};
```

### 错误码
```cpp
enum class ErrorCode {
    SUCCESS = 0,
    NODE_NOT_FOUND = 1001,
    PACKET_NOT_FOUND = 1002,
    PORT_CONNECTION_FAILED = 1003,
    SERIALIZATION_FAILED = 1004,
    DESERIALIZATION_FAILED = 1005,
    TYPE_ID_COLLISION = 1006,
    NETWORK_NOT_RUNNING = 1007,
    INVALID_PARAMETER = 1008
};
```

## 配置系统

### 配置文件格式
```json
{
    "simulation": {
        "mode": "STEP",
        "parallelization": "THREAD_POOL",
        "serialization": "JSON",
        "thread_count": 4
    },
    "network": {
        "name": "TestNetwork",
        "nodes": [
            {
                "type": "ProducerNode",
                "id": 1,
                "name": "Producer1",
                "position": {"x": 0, "y": 0},
                "parameters": {
                    "production_rate": 1
                }
            }
        ],
        "connections": [
            {
                "from": {"node_id": 1, "port": "output"},
                "to": {"node_id": 2, "port": "input"}
            }
        ]
    }
}
```

## 性能监控

### 性能指标API
```cpp
class PerformanceMonitor {
public:
    // 时间测量
    void StartTimer(const std::string& name);
    void EndTimer(const std::string& name);
    double GetAverageTime(const std::string& name) const;
    
    // 内存监控
    size_t GetMemoryUsage() const;
    size_t GetPeakMemoryUsage() const;
    
    // 网络统计
    size_t GetPacketsProcessed() const;
    double GetPacketsPerSecond() const;
    size_t GetActiveNodes() const;
    
    // 报告生成
    nlohmann::json GenerateReport() const;
    void SaveReport(const std::string& filename) const;
};
``` 