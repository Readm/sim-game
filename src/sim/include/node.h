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

class Node {
public:
    Node(NodeID id = 0, TypeID packet_type_id = 0) 
        : node_id_(id), packet_type_id_(packet_type_id), tick_tock_(0), next_packet_seq_(1) {
        // 创建线程池单例
        if (parallelization_method_ == ParallelizationMethod::THREAD_POOL && !thread_pool_) {
            thread_pool_ = std::make_shared<ThreadPool>();
        }
    }
    virtual ~Node() = default;

    // 获取Node的TypeID
    virtual TypeID getTypeID() const = 0;

    // 获取Node的ID
    NodeID getNodeID() const { return node_id_; }

    // 获取该节点可以生成的Packet类型ID
    TypeID getPacketTypeID() const { return packet_type_id_; }

    // 生成新的PacketID
    PacketID generateNextPacketID() {
        uint64_t seq = next_packet_seq_++;  // 先获取当前值并递增
        return PacketID(node_id_, seq);     // 使用递增前的值创建ID
    }

    // 生成新的Packet
    template<typename T>
    std::shared_ptr<T> spawnPacket() {
        static_assert(std::is_base_of<Packet, T>::value, "T must be derived from Packet");
        if (T::type_id != packet_type_id_) {
            return nullptr;  // 该节点不能生成这种类型的Packet
        }
        PacketID id = generateNextPacketID();  // 先生成ID
        return std::shared_ptr<T>(new T(node_id_, id));  // 使用生成的ID创建Packet
    }

    // 子节点管理
    void addChild(std::shared_ptr<Node> child) {
        children_.push_back(child);
    }

    const std::vector<std::shared_ptr<Node>>& getChildren() const {
        return children_;
    }

    // Packet缓冲区管理
    void addPacket(std::shared_ptr<Packet> packet) {
        buffer_.push_back(packet);
    }

    const std::vector<std::shared_ptr<Packet>>& getBuffer() const {
        return buffer_;
    }

    void clearBuffer() {
        buffer_.clear();
    }

    // Port管理
    std::shared_ptr<InputPort> addInputPort(const std::string& name, TypeID accepted_type_id, size_t capacity = 0) {
        auto port = std::make_shared<InputPort>(name, accepted_type_id, capacity);
        input_ports_[name] = port;
        return port;
    }

    std::shared_ptr<OutputPort> addOutputPort(const std::string& name, TypeID accepted_type_id, size_t capacity = 0) {
        auto port = std::make_shared<OutputPort>(name, accepted_type_id, capacity);
        output_ports_[name] = port;
        return port;
    }

    std::shared_ptr<InputPort> getInputPort(const std::string& name) const {
        auto it = input_ports_.find(name);
        return it != input_ports_.end() ? it->second : nullptr;
    }

    std::shared_ptr<OutputPort> getOutputPort(const std::string& name) const {
        auto it = output_ports_.find(name);
        return it != output_ports_.end() ? it->second : nullptr;
    }

    const std::unordered_map<std::string, std::shared_ptr<InputPort>>& getInputPorts() const {
        return input_ports_;
    }

    const std::unordered_map<std::string, std::shared_ptr<OutputPort>>& getOutputPorts() const {
        return output_ports_;
    }

    // TickTock系统
    uint64_t getTickTock() const { return tick_tock_; }

    // 设置并行化方法
    static void setParallelizationMethod(ParallelizationMethod method) {
        parallelization_method_ = method;
        if (method == ParallelizationMethod::THREAD_POOL && !thread_pool_) {
            thread_pool_ = std::make_shared<ThreadPool>();
        }
    }

    // Tick：只读取状态，不修改状态
    virtual void tick() {
        #if SIM_BUILD_MODE == SIM_DEBUG_MODE
        auto pre_tick_state = serialize();
        #endif

        onTick();

        if (parallelization_method_ == ParallelizationMethod::THREAD_POOL && thread_pool_) {
            std::vector<std::future<void>> futures;
            for (auto& child : children_) {
                futures.push_back(thread_pool_->enqueue([&child] {
                    child->tick();
                }));
            }
            // 等待所有子节点完成tick
            for (auto& future : futures) {
                future.wait();
            }
        } else {
            // 串行执行
            for (auto& child : children_) {
                child->tick();
            }
        }

        #if SIM_BUILD_MODE == SIM_DEBUG_MODE
        auto post_tick_state = serialize();
        if (pre_tick_state != post_tick_state) {
            SIM_ERROR("Node state changed during tick operation");
        }
        #endif
    }

    // Tock：执行状态更新
    virtual void tock() {
        onTock();

        if (parallelization_method_ == ParallelizationMethod::THREAD_POOL && thread_pool_) {
            std::vector<std::future<void>> futures;
            for (auto& child : children_) {
                futures.push_back(thread_pool_->enqueue([&child] {
                    child->tock();
                }));
            }
            // 等待所有子节点完成tock
            for (auto& future : futures) {
                future.wait();
            }
        } else {
            // 串行执行
            for (auto& child : children_) {
                child->tock();
            }
        }

        ++tick_tock_;
    }

    // 序列化接口
    virtual std::string serialize(SerializationMethod method = SerializationMethod::JSON) const final {
        switch (method) {
            case SerializationMethod::JSON: {
                nlohmann::json j;
                j["type_id"] = getTypeID();
                j["node_id"] = node_id_;
                j["tick_tock"] = tick_tock_;
                
                // 序列化子节点
                nlohmann::json children_json;
                for (const auto& child : children_) {
                    children_json.push_back(nlohmann::json::parse(child->serialize()));
                }
                j["children"] = children_json;

                // 序列化缓冲区
                nlohmann::json buffer_json;
                for (const auto& packet : buffer_) {
                    buffer_json.push_back(nlohmann::json::parse(packet->serialize()));
                }
                j["buffer"] = buffer_json;

                // 序列化端口
                nlohmann::json input_ports_json;
                for (const auto& [name, port] : input_ports_) {
                    input_ports_json[name] = nlohmann::json::parse(port->serialize());
                }
                j["input_ports"] = input_ports_json;

                nlohmann::json output_ports_json;
                for (const auto& [name, port] : output_ports_) {
                    output_ports_json[name] = nlohmann::json::parse(port->serialize());
                }
                j["output_ports"] = output_ports_json;

                serializeImpl(j);
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
                node_id_ = j["node_id"];
                tick_tock_ = j["tick_tock"];
                
                // 反序列化子节点
                children_.clear();
                for (const auto& child_json : j["children"]) {
                    auto child = createNodeFromJson(child_json);
                    if (child) {
                        children_.push_back(child);
                    }
                }

                // 反序列化缓冲区
                buffer_.clear();
                for (const auto& packet_json : j["buffer"]) {
                    auto packet = createPacketFromJson(packet_json);
                    if (packet) {
                        buffer_.push_back(packet);
                    }
                }

                // 反序列化端口
                input_ports_.clear();
                for (const auto& [name, port_json] : j["input_ports"].items()) {
                    auto port = std::make_shared<InputPort>(name, port_json["accepted_type_id"], port_json["capacity"]);
                    port->deserialize(port_json.dump());
                    input_ports_[name] = port;
                }

                output_ports_.clear();
                for (const auto& [name, port_json] : j["output_ports"].items()) {
                    auto port = std::make_shared<OutputPort>(name, port_json["accepted_type_id"], port_json["capacity"]);
                    port->deserialize(port_json.dump());
                    output_ports_[name] = port;
                }

                deserializeImpl(j);
                break;
            }
            case SerializationMethod::BINARY:
                // TODO: 实现二进制反序列化
                throw std::runtime_error("Binary deserialization not implemented yet");
            case SerializationMethod::PROTOBUF:
                // TODO: 实现protobuf反序列化
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
    virtual void serializeImpl(nlohmann::json& j) const {}
    virtual void deserializeImpl(const nlohmann::json& j) {}

    // 子类需要实现的Tick和Tock操作
    virtual void onTick() {}
    virtual void onTock() {}

    NodeID node_id_;
    TypeID packet_type_id_;  // 该节点可以生成的Packet类型ID
    uint64_t tick_tock_;
    uint64_t next_packet_seq_;  // 本地Packet序列号计数器
    std::vector<std::shared_ptr<Node>> children_;
    std::vector<std::shared_ptr<Packet>> buffer_;
    std::unordered_map<std::string, std::shared_ptr<InputPort>> input_ports_;
    std::unordered_map<std::string, std::shared_ptr<OutputPort>> output_ports_;

    // 静态成员用于并行化控制
    static inline ParallelizationMethod parallelization_method_ = ParallelizationMethod::NONE;
    static inline std::shared_ptr<ThreadPool> thread_pool_ = nullptr;

private:
    virtual std::shared_ptr<Node> createNodeFromJson(const nlohmann::json& j) = 0;
    virtual std::shared_ptr<Packet> createPacketFromJson(const nlohmann::json& j) = 0;

    // 最快模式：不断simulate直到结束
    void simulateFastest(uint64_t duration, std::ostream& out) {
        uint64_t target_tick = tick_tock_ + duration;
        while (tick_tock_ < target_tick) {
            tick();
            tock();
        }
        // 在结束时输出最终状态
        out << serialize() << std::endl;
    }

    // 单步模式：每次执行一个Tick或Tock，等待输入
    void simulateStep(uint64_t duration, std::istream& in, std::ostream& out) {
        uint64_t target_tick = tick_tock_ + duration;
        std::string input;
        while (tick_tock_ < target_tick) {
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
        uint64_t target_tick = tick_tock_ + duration;
        while (tick_tock_ < target_tick) {
            tick();
            tock();
            // 每个Tock后输出状态
            out << serialize() << std::endl;
        }
    }
};

} // namespace sim 