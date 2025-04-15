/**
 * @file port.h
 * @brief 定义了仿真系统中的端口系统
 * 
 * 该文件实现了一个完整的端口系统，包括：
 * - Port基类：提供基本的端口功能和状态管理
 * - InputPort：输入端口，用于接收数据包
 * - OutputPort：输出端口，用于发送数据包
 * 
 * 端口系统特性：
 * - 支持类型检查和容量限制
 * - 实现了Tick-Tock时序机制
 * - 支持序列化和反序列化
 * - 支持端口之间的动态连接
 * - 实现了基于valid-ready握手协议
 */

#pragma once

#include "type.h"
#include "packet.h"
#include <memory>
#include <vector>
#include <string>
#include <optional>

namespace sim {

// 前向声明
class Node;

/**
 * @brief 端口基类，提供数据包传输的基本功能
 * 
 * Port类实现了以下功能：
 * - 基本的端口属性管理
 * - 数据包类型检查
 * - 容量管理
 * - Tick-Tock时序支持
 * - 序列化/反序列化
 */
class Port {
public:
    /**
     * @brief 端口的持久状态结构
     * 
     * 包含需要在Tock阶段更新并需要序列化的状态：
     * - 端口名称和类型信息
     * - 容量限制
     * - 时序计数器
     * - 数据包缓冲区
     */
    struct PersistentState {
        std::string name;
        TypeID accepted_type_id;
        size_t capacity;
        uint64_t tick_tock;
        std::vector<std::shared_ptr<Packet>> packets;

        void serialize(nlohmann::json& j) const {
            j["name"] = name;
            j["accepted_type_id"] = accepted_type_id;
            j["capacity"] = capacity;
            j["tick_tock"] = tick_tock;
            
            nlohmann::json packets_json;
            for (const auto& packet : packets) {
                packets_json.push_back(nlohmann::json::parse(packet->serialize()));
            }
            j["packets"] = packets_json;
        }

        void deserialize(const nlohmann::json& j) {
            name = j["name"];
            accepted_type_id = j["accepted_type_id"];
            capacity = j["capacity"];
            tick_tock = j["tick_tock"];
        }
    };

    /**
     * @brief 端口的临时状态结构
     * 
     * 包含在Tick阶段更新但不需要序列化的状态：
     * - 更新标志
     */
    struct TransientState {
        bool can_update = false;
    };

    /**
     * @brief 构造一个新的端口
     * @param name 端口名称
     * @param accepted_type_id 端口接受的数据包类型ID
     * @param capacity 端口容量，0表示无限容量
     */
    Port(const std::string& name, TypeID accepted_type_id, size_t capacity = 0) {
        p_state_.name = name;
        p_state_.accepted_type_id = accepted_type_id;
        p_state_.capacity = capacity;
        p_state_.tick_tock = 0;
    }
    virtual ~Port() = default;

    // 基本属性
    /**
     * @brief 获取端口名称
     * @return 端口名称的常引用
     */
    const std::string& getName() const { return p_state_.name; }
    /**
     * @brief 获取端口接受的数据包类型ID
     * @return 数据包类型ID
     */
    TypeID getAcceptedTypeID() const { return p_state_.accepted_type_id; }
    size_t getCapacity() const { return p_state_.capacity; }
    /**
     * @brief 检查端口是否还有容量
     * @return true如果端口未满，false如果端口已满
     */
    bool hasCapacity() const { return p_state_.capacity == 0 || p_state_.packets.size() < p_state_.capacity; }
    size_t size() const { return p_state_.packets.size(); }

    /**
     * @brief 检查数据包类型是否匹配
     * @param packet 要检查的数据包
     * @return true如果数据包类型匹配，false如果不匹配
     */
    bool canAcceptPacket(const Packet& packet) const {
        return packet.getTypeID() == p_state_.accepted_type_id;
    }

    // TickTock系统
    uint64_t getTickTock() const { return p_state_.tick_tock; }
    
    // 在Tick阶段检查状态
    virtual void tick() {
        onTick();
    }
    
    // 在Tock阶段更新状态
    virtual void tock() {
        onTock();
        p_state_.tick_tock++;
    }

    // 序列化接口
    virtual std::string serialize(SerializationMethod method = SerializationMethod::JSON) const final {
        switch (method) {
            case SerializationMethod::JSON: {
                nlohmann::json j;
                p_state_.serialize(j);
                serializeImpl(j);
                return j.dump();
            }
            case SerializationMethod::BINARY:
                throw std::runtime_error("Binary serialization not implemented yet");
            case SerializationMethod::PROTOBUF:
                throw std::runtime_error("Protobuf serialization not implemented yet");
            default:
                throw std::runtime_error("Unknown serialization method");
        }
    }

    virtual void deserialize(const std::string& data, SerializationMethod method = SerializationMethod::JSON) final {
        switch (method) {
            case SerializationMethod::JSON: {
                auto j = nlohmann::json::parse(data);
                p_state_.deserialize(j);
                deserializeImpl(j);
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

protected:
    // 子类可以重写这些方法来添加自己的序列化逻辑
    virtual void serializeImpl(nlohmann::json& j) const {}
    virtual void deserializeImpl(const nlohmann::json& j) {}

    // 子类需要实现的Tick和Tock操作
    virtual void onTick() {}
    virtual void onTock() {}

    PersistentState p_state_;
    TransientState t_state_;
};

/**
 * @brief 输入端口类，用于接收数据包
 * 
 * 实现了以下功能：
 * - 数据包接收和缓存
 * - valid信号生成
 * - FIFO队列管理
 */
class InputPort : public Port {
public:
    using Port::Port;

    /**
     * @brief 检查是否有数据可以发送
     * @return true如果有数据包待处理，false如果端口为空
     */
    bool isValid() const {
        return !p_state_.packets.empty();
    }

    /**
     * @brief 接收一个数据包
     * @param packet 要接收的数据包
     * @return true如果接收成功，false如果接收失败（类型不匹配或端口已满）
     */
    bool receivePacket(std::shared_ptr<Packet> packet) {
        if (!packet || !canAcceptPacket(*packet) || !hasCapacity()) {
            return false;
        }
        p_state_.packets.push_back(packet);
        return true;
    }

    // 获取并移除第一个数据包
    std::shared_ptr<Packet> popPacket() {
        if (!t_state_.can_update || p_state_.packets.empty()) {
            return nullptr;
        }
        auto packet = p_state_.packets.front();
        p_state_.packets.erase(p_state_.packets.begin());
        return packet;
    }

    // 查看第一个数据包但不移除
    std::shared_ptr<Packet> peekPacket() const {
        return p_state_.packets.empty() ? nullptr : p_state_.packets.front();
    }

protected:
    void onTick() override {
        // 在Tick阶段，检查是否有数据可以发送
        t_state_.can_update = isValid();
    }
};

/**
 * @brief 输出端口类，用于发送数据包
 * 
 * 实现了以下功能：
 * - 数据包广播发送
 * - 动态端口连接管理
 * - ready信号生成
 */
class OutputPort : public Port {
public:
    /**
     * @brief 输出端口的临时状态结构
     * 
     * 包含连接的输入端口列表
     */
    struct OutputTransientState : TransientState {
        std::vector<std::shared_ptr<InputPort>> connected_ports;
    };

    OutputPort(const std::string& name, TypeID accepted_type_id, size_t capacity = 0)
        : Port(name, accepted_type_id, capacity) {}

    // 连接到输入端口
    void connectTo(std::shared_ptr<InputPort> input_port) {
        t_state_.connected_ports.push_back(input_port);
    }

    // 断开与输入端口的连接
    void disconnectFrom(std::shared_ptr<InputPort> input_port) {
        auto it = std::find(t_state_.connected_ports.begin(), t_state_.connected_ports.end(), input_port);
        if (it != t_state_.connected_ports.end()) {
            t_state_.connected_ports.erase(it);
        }
    }

    // 检查是否ready可以接收数据（ready信号）
    virtual bool isReady() const {
        return hasCapacity();
    }

    // 发送数据包到所有连接的输入端口
    bool sendPacket(std::shared_ptr<Packet> packet) {
        if (!t_state_.can_update || !packet || !canAcceptPacket(*packet)) {
            return false;
        }

        bool sent = false;
        for (auto& input_port : t_state_.connected_ports) {
            if (input_port && input_port->hasCapacity()) {
                input_port->receivePacket(packet);
                sent = true;
            }
        }
        return sent;
    }

    // 检查是否所有连接的输入端口都已满
    bool areAllInputPortsFull() const {
        for (const auto& input_port : t_state_.connected_ports) {
            if (input_port && input_port->hasCapacity()) {
                return false;
            }
        }
        return true;
    }

    // 获取连接的输入端口
    const std::vector<std::shared_ptr<InputPort>>& getConnectedPorts() const {
        return t_state_.connected_ports;
    }

protected:
    void onTick() override {
        // 在Tick阶段，检查是否可以接收数据
        t_state_.can_update = isReady();
        
        // 检查所有连接的输入端口的valid信号
        for (const auto& input_port : t_state_.connected_ports) {
            if (input_port && input_port->isValid()) {
                t_state_.can_update &= true;  // 如果有任何一个输入端口有数据，就可以更新
            }
        }
    }

private:
    OutputTransientState t_state_;
};

} // namespace sim 