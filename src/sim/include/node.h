/**
 * @file node.h
 * @brief 定义了仿真系统中的基础节点类
 * 
 * Node类是仿真系统中的核心组件，它实现了以下功能：
 * - 支持Tick-Tock仿真机制
 * - 管理节点的持久状态和临时状态
 * - 提供数据包的生成和管理功能
 * - 支持输入/输出端口系统
 * - 实现子节点的树形结构
 * - 提供序列化和反序列化功能
 * - 支持多种仿真模式（最快模式、单步模式、跟踪模式）
 * - 支持并行化处理（线程池）
 */

#pragma once

#include "type.h"
#include "packet.h"
#include "port.h"
#include "common.h"
#include "thread_pool.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
#include <future>
#include <iostream>

namespace sim {

/**
 * @brief 仿真系统中的基础节点类
 * 
 * Node类是仿真系统的基本构建块，实现了以下功能：
 * - 状态管理（持久状态和临时状态）
 * - 数据包生成和处理
 * - 端口管理
 * - 子节点管理
 * - 序列化/反序列化
 */
class Node {
public:
    /**
     * @brief 节点的持久状态结构
     * 
     * 包含需要在Tock阶段更新并需要序列化的状态：
     * - 节点标识和类型信息
     * - 时序计数器
     * - 数据包序列号
     * - 子节点和缓冲区
     * - 输入/输出端口
     */
    struct PersistentState {
        NodeID node_id;
        TypeID packet_type_id;  // 该节点可以生成的Packet类型ID
        uint64_t tick_tock;
        uint64_t next_packet_seq;  // 本地Packet序列号计数器
        std::vector<std::shared_ptr<Node>> children;
        std::vector<std::shared_ptr<Packet>> buffer;
        size_t buffer_capacity = 100;  // 添加buffer容量限制，默认100
        std::unordered_map<std::string, std::shared_ptr<InputPort>> input_ports;
        std::unordered_map<std::string, std::shared_ptr<OutputPort>> output_ports;

        /**
         * @brief 节点的显示状态结构
         * 
         * 包含节点的显示相关状态：
         * - 是否展开
         * - 显示坐标
         */
        struct DisplayState {
            bool is_expanded = false;  // 是否展开
            int x = -1;               // x坐标，-1表示未指定
            int y = -1;               // y坐标，-1表示未指定

            void serialize(nlohmann::json& j) const {
                j["is_expanded"] = is_expanded;
                j["x"] = x;
                j["y"] = y;
            }

            void deserialize(const nlohmann::json& j) {
                is_expanded = j["is_expanded"];
                x = j["x"];
                y = j["y"];
            }
        };

        DisplayState d_state;  // 显示状态

        void serialize(nlohmann::json& j) const {
            j["node_id"] = node_id;
            j["tick_tock"] = tick_tock;
            j["packet_type_id"] = packet_type_id;
            j["next_packet_seq"] = next_packet_seq;
            j["buffer_capacity"] = buffer_capacity;  // 序列化buffer容量
            
            // 序列化显示状态
            nlohmann::json d_state_json;
            d_state.serialize(d_state_json);
            j["d_state"] = d_state_json;
            
            // 序列化子节点
            nlohmann::json children_json;
            for (const auto& child : children) {
                children_json.push_back(nlohmann::json::parse(child->serialize()));
            }
            j["children"] = children_json;

            // 序列化缓冲区
            nlohmann::json buffer_json;
            for (const auto& packet : buffer) {
                buffer_json.push_back(nlohmann::json::parse(packet->serialize()));
            }
            j["buffer"] = buffer_json;

            // 序列化端口
            nlohmann::json input_ports_json;
            for (const auto& [name, port] : input_ports) {
                input_ports_json[name] = nlohmann::json::parse(port->serialize());
            }
            j["input_ports"] = input_ports_json;

            nlohmann::json output_ports_json;
            for (const auto& [name, port] : output_ports) {
                output_ports_json[name] = nlohmann::json::parse(port->serialize());
            }
            j["output_ports"] = output_ports_json;
        }

        void deserialize(const nlohmann::json& j, Node* node) {
            node_id = j["node_id"];
            tick_tock = j["tick_tock"];
            packet_type_id = j["packet_type_id"];
            next_packet_seq = j["next_packet_seq"];
            if (j.contains("buffer_capacity")) {
                buffer_capacity = j["buffer_capacity"];
            }

            // 反序列化显示状态
            if (j.contains("d_state")) {
                d_state.deserialize(j["d_state"]);
            }

            // 反序列化子节点
            children.clear();
            for (const auto& child_json : j["children"]) {
                auto child = node->createNodeFromJson(child_json);
                if (child) {
                    children.push_back(child);
                }
            }

            // 反序列化缓冲区
            buffer.clear();
            for (const auto& packet_json : j["buffer"]) {
                auto packet = node->createPacketFromJson(packet_json);
                if (packet) {
                    buffer.push_back(packet);
                }
            }

            // 反序列化端口
            input_ports.clear();
            for (const auto& [name, port_json] : j["input_ports"].items()) {
                auto port = std::make_shared<InputPort>(name, port_json["accepted_type_id"], port_json["capacity"]);
                port->deserialize(port_json.dump());
                input_ports[name] = port;
            }

            output_ports.clear();
            for (const auto& [name, port_json] : j["output_ports"].items()) {
                auto port = std::make_shared<OutputPort>(name, port_json["accepted_type_id"], port_json["capacity"]);
                port->deserialize(port_json.dump());
                output_ports[name] = port;
            }
        }
    };

    /**
     * @brief 节点的临时状态结构
     * 
     * 包含在Tick阶段更新但不需要序列化的状态
     */
    struct TransientState {
        // 目前没有临时状态，但为了扩展性保留此结构
    };

    /**
     * @brief 构造一个新的节点
     * @param id 节点的唯一标识符
     * @param packet_type_id 该节点可以生成的数据包类型ID
     */
    Node(NodeID id = 0, TypeID packet_type_id = 0) {
        p_state_.node_id = id;
        p_state_.packet_type_id = packet_type_id;
        p_state_.tick_tock = 0;
        p_state_.next_packet_seq = 1;

        // 创建线程池单例
        if (parallelization_method_ == ParallelizationMethod::THREAD_POOL && !thread_pool_) {
            thread_pool_ = std::make_shared<ThreadPool>();
        }
    }
    virtual ~Node() = default;

    /**
     * @brief 获取节点的类型ID
     * @return 节点的类型ID
     */
    virtual TypeID getTypeID() const = 0;

    /**
     * @brief 获取节点的唯一标识符
     * @return 节点ID
     */
    NodeID getNodeID() const { return p_state_.node_id; }

    /**
     * @brief 获取该节点可以生成的数据包类型ID
     * @return 数据包类型ID
     */
    TypeID getPacketTypeID() const { return p_state_.packet_type_id; }

    /**
     * @brief 生成新的数据包ID
     * @return 全局唯一的数据包ID
     */
    PacketID generateNextPacketID() {
        uint64_t seq = p_state_.next_packet_seq++;  // 先获取当前值并递增
        return PacketID(p_state_.node_id, seq);     // 使用递增前的值创建ID
    }

    /**
     * @brief 生成新的数据包
     * @tparam T 数据包类型，必须继承自Packet
     * @return 新创建的数据包的智能指针，如果类型不匹配则返回nullptr
     */
    template<typename T>
    std::shared_ptr<T> spawnPacket() {
        static_assert(std::is_base_of<Packet, T>::value, "T must be derived from Packet");
        if (T::type_id != p_state_.packet_type_id) {
            return nullptr;  // 该节点不能生成这种类型的Packet
        }
        PacketID id = generateNextPacketID();  // 先生成ID
        return std::shared_ptr<T>(new T(p_state_.node_id, id));  // 使用生成的ID创建Packet
    }

    /**
     * @brief 添加子节点
     * @param child 要添加的子节点
     */
    void addChild(std::shared_ptr<Node> child) {
        p_state_.children.push_back(child);
    }

    /**
     * @brief 获取所有子节点
     * @return 子节点列表的常引用
     */
    const std::vector<std::shared_ptr<Node>>& getChildren() const {
        return p_state_.children;
    }

    // Packet缓冲区管理
    bool addPacket(std::shared_ptr<Packet> packet) {
        if (p_state_.buffer.size() >= p_state_.buffer_capacity) {
            return false;
        }
        p_state_.buffer.push_back(packet);
        return true;
    }

    const std::vector<std::shared_ptr<Packet>>& getBuffer() const {
        return p_state_.buffer;
    }

    void clearBuffer() {
        p_state_.buffer.clear();
    }

    /**
     * @brief 添加输入端口
     * @param name 端口名称
     * @param accepted_type_id 端口接受的数据包类型ID
     * @param capacity 端口容量，0表示无限容量
     * @return 创建的输入端口的智能指针
     */
    std::shared_ptr<InputPort> addInputPort(const std::string& name, TypeID accepted_type_id, size_t capacity = 0) {
        auto port = std::make_shared<InputPort>(name, accepted_type_id, capacity);
        p_state_.input_ports[name] = port;
        return port;
    }

    /**
     * @brief 添加输出端口
     * @param name 端口名称
     * @param accepted_type_id 端口接受的数据包类型ID
     * @param capacity 端口容量，0表示无限容量
     * @return 创建的输出端口的智能指针
     */
    std::shared_ptr<OutputPort> addOutputPort(const std::string& name, TypeID accepted_type_id, size_t capacity = 0) {
        auto port = std::make_shared<OutputPort>(name, accepted_type_id, capacity);
        p_state_.output_ports[name] = port;
        return port;
    }

    /**
     * @brief 获取输入端口
     * @param name 端口名称
     * @return 输入端口的智能指针，如果端口不存在则返回nullptr
     */
    std::shared_ptr<InputPort> getInputPort(const std::string& name) {
        auto it = p_state_.input_ports.find(name);
        return it != p_state_.input_ports.end() ? it->second : nullptr;
    }

    /**
     * @brief 获取输出端口
     * @param name 端口名称
     * @return 输出端口的智能指针，如果端口不存在则返回nullptr
     */
    std::shared_ptr<OutputPort> getOutputPort(const std::string& name) {
        auto it = p_state_.output_ports.find(name);
        return it != p_state_.output_ports.end() ? it->second : nullptr;
    }
    
    /**
     * @brief 获取所有输入端口
     * @return 输入端口映射表的常引用
     */
    const std::unordered_map<std::string, std::shared_ptr<InputPort>>& getInputPorts() const {
        return p_state_.input_ports;
    }
    
    /**
     * @brief 获取所有输出端口
     * @return 输出端口映射表的常引用
     */
    const std::unordered_map<std::string, std::shared_ptr<OutputPort>>& getOutputPorts() const {
        return p_state_.output_ports;
    }

    /**
     * @brief 获取当前的Tick-Tock计数
     * @return Tick-Tock计数
     */
    uint64_t getTickTock() const { return p_state_.tick_tock; }

    /**
     * @brief 获取节点的buffer容量
     * @return buffer容量
     */
    size_t getBufferCapacity() const { return p_state_.buffer_capacity; }

    /**
     * @brief 设置节点的buffer容量
     * @param capacity 新的buffer容量
     */
    void setBufferCapacity(size_t capacity) { p_state_.buffer_capacity = capacity; }

    // 设置并行化方法
    static void setParallelizationMethod(ParallelizationMethod method) {
        parallelization_method_ = method;
        if (method == ParallelizationMethod::THREAD_POOL && !thread_pool_) {
            thread_pool_ = std::make_shared<ThreadPool>();
        }
    }
    
    /**
     * @brief 获取节点的持久状态
     * @return 节点持久状态的常引用
     */
    const PersistentState& getPersistentState() const {
        return p_state_;
    }

    /**
     * @brief 获取节点的显示状态
     * @return 显示状态的常引用
     */
    const PersistentState::DisplayState& getDisplayState() const {
        return p_state_.d_state;
    }

    /**
     * @brief 设置节点的显示状态
     * @param state 新的显示状态
     */
    void setDisplayState(const PersistentState::DisplayState& state) {
        p_state_.d_state = state;
    }
    // 在Tick阶段执行计算
    virtual void tick() {
        // 先保存状态，用于用户控制
        auto pre_tick_state = serialize();

        // 处理输入端口
        for (auto& [name, port] : p_state_.input_ports) {
            port->tick();
        }

        // 处理子节点的Tick
        if (parallelization_method_ == ParallelizationMethod::THREAD_POOL && thread_pool_) {
            std::vector<std::future<void>> futures;
            for (auto& child : p_state_.children) {
                // 并行处理
                futures.push_back(thread_pool_->enqueue([&child](){ child->tick(); }));
            }
            // 等待所有任务完成
            for (auto& future : futures) {
                future.get();
            }
        } else {
            // 串行处理
            for (auto& child : p_state_.children) {
                child->tick();
            }
        }

        // 处理输出端口
        for (auto& [name, port] : p_state_.output_ports) {
            port->tick();
        }

        // 执行节点特定的Tick操作
        onTick();
    }

    // 在Tock阶段更新状态
    virtual void tock() {
        // 先保存状态，用于用户控制
        auto pre_tock_state = serialize();

        // 处理输入端口
        for (auto& [name, port] : p_state_.input_ports) {
            port->tock();
        }

        // 处理子节点的Tock
        if (parallelization_method_ == ParallelizationMethod::THREAD_POOL && thread_pool_) {
            std::vector<std::future<void>> futures;
            for (auto& child : p_state_.children) {
                // 并行处理
                futures.push_back(thread_pool_->enqueue([&child](){ child->tock(); }));
            }
            // 等待所有任务完成
            for (auto& future : futures) {
                future.get();
            }
        } else {
            // 串行处理
            for (auto& child : p_state_.children) {
                child->tock();
            }
        }

        // 处理输出端口
        for (auto& [name, port] : p_state_.output_ports) {
            port->tock();
        }

        // 更新Tick-Tock计数器
        p_state_.tick_tock++;

        // 执行节点特定的Tock操作
        onTock();
    }

    // 序列化接口
    virtual std::string serialize(SerializationMethod method = SerializationMethod::JSON) const final {
        switch (method) {
            case SerializationMethod::JSON: {
                nlohmann::json j;
                p_state_.serialize(j);
                j["type_id"] = getTypeID();  // 添加类型ID
                serializeImpl(j);  // 调用子类的额外序列化
                return j.dump();
            }
            case SerializationMethod::BINARY:
                // TODO: 实现二进制序列化
                throw std::runtime_error("Binary serialization not implemented yet");
            case SerializationMethod::PROTOBUF:
                // TODO: 实现protobuf序列化
                throw std::runtime_error("Protobuf serialization not implemented yet");
            default:
                throw std::runtime_error("Unknown serialization method");
        }
    }

    virtual void deserialize(const std::string& data, SerializationMethod method = SerializationMethod::JSON) final {
        switch (method) {
            case SerializationMethod::JSON: {
                auto j = nlohmann::json::parse(data);
                p_state_.deserialize(j, this);
                deserializeImpl(j);  // 调用子类的额外反序列化
                break;
            }
            case SerializationMethod::BINARY:
                throw std::runtime_error("Binary deserialization not implemented yet");
            case SerializationMethod::PROTOBUF:
                throw std::runtime_error("Protobuf deserialization not implemented yet");
            default:
                throw std::runtime_error("Unknown serialization method");
        }
    }

    // 模拟函数
    void simulate(SimulationMode mode, uint64_t duration, std::istream& in = std::cin, std::ostream& out = std::cout) {
        switch (mode) {
            case SimulationMode::FASTEST:
                simulateFastest(duration, out);
                break;
            case SimulationMode::STEP:
                simulateStep(duration, in, out);
                break;
            case SimulationMode::TRACE:
                simulateTrace(duration, out);
                break;
        }
    }

protected:
    // 子类可以重写这些方法来添加自己的序列化逻辑
    virtual void serializeImpl([[maybe_unused]] nlohmann::json& j) const {}
    virtual void deserializeImpl([[maybe_unused]] const nlohmann::json& j) {}

    // 子类需要实现的Tick和Tock操作
    virtual void onTick() {}
    virtual void onTock() {}

    // 子类需要实现的工厂方法
    virtual std::shared_ptr<Node> createNodeFromJson(const nlohmann::json& j) = 0;
    virtual std::shared_ptr<Packet> createPacketFromJson(const nlohmann::json& j) = 0;

    PersistentState p_state_;
    TransientState t_state_;

    // 静态成员用于并行化控制
    static inline ParallelizationMethod parallelization_method_ = ParallelizationMethod::NONE;
    static inline std::shared_ptr<ThreadPool> thread_pool_ = nullptr;

private:
    // 最快模式：不断simulate直到结束
    void simulateFastest(uint64_t duration, std::ostream& out) {
        uint64_t target_tick = p_state_.tick_tock + duration;
        while (p_state_.tick_tock < target_tick) {
            tick();
            tock();
        }
        // 在结束时输出最终状态
        out << serialize() << std::endl;
    }

    // 单步模式：每次执行一个Tick或Tock，等待输入
    void simulateStep(uint64_t duration, std::istream& in, std::ostream& out) {
        uint64_t target_tick = p_state_.tick_tock + duration;
        std::string input;
        while (p_state_.tick_tock < target_tick) {
            // 执行Tick并输出状态
            out << "Before Tick (Press Enter to continue):" << std::endl;
            out << serialize() << std::endl;
            std::getline(in, input);
            
            tick();
            
            // 执行Tock并输出状态
            out << "Before Tock (Press Enter to continue):" << std::endl;
            out << serialize() << std::endl;
            std::getline(in, input);
            
            tock();
        }
        // 输出最终状态
        out << "Final state:" << std::endl;
        out << serialize() << std::endl;
    }

    // 跟踪模式：每个Tock后输出序列化结果
    void simulateTrace(uint64_t duration, std::ostream& out) {
        uint64_t target_tick = p_state_.tick_tock + duration;
        while (p_state_.tick_tock < target_tick) {
            tick();
            tock();
            // 每个Tock后输出状态
            out << serialize() << std::endl;
        }
    }
};

} // namespace sim 