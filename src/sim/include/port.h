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
 */
class Port {
public:
    /**
     * @brief 端口的持久状态结构
     */
    struct PersistentState {
        std::string name;
        TypeID accepted_type_id;
        size_t capacity;
        uint64_t tick_tock;
        std::vector<std::shared_ptr<Packet>> packets;
        bool valid = false;  // 表示是否有数据可以发送
        bool ready = false;  // 表示是否可以接收数据

        void serialize(nlohmann::json& j) const {
            j["name"] = name;
            j["accepted_type_id"] = accepted_type_id;
            j["capacity"] = capacity;
            j["tick_tock"] = tick_tock;
            j["valid"] = valid;
            j["ready"] = ready;
            
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
            valid = j["valid"];
            ready = j["ready"];
        }
    };

    Port(const std::string& name, TypeID accepted_type_id, size_t capacity = 0) {
        p_state_.name = name;
        p_state_.accepted_type_id = accepted_type_id;
        p_state_.capacity = capacity;
        p_state_.tick_tock = 0;
    }
    virtual ~Port() = default;

    // 基本属性
    const std::string& getName() const { return p_state_.name; }
    TypeID getAcceptedTypeID() const { return p_state_.accepted_type_id; }
    size_t getCapacity() const { return p_state_.capacity; }
    bool hasCapacity() const { return p_state_.capacity == 0 || p_state_.packets.size() < p_state_.capacity; }
    size_t size() const { return p_state_.packets.size(); }
    bool isValid() const { return p_state_.valid; }
    bool isReady() const { return p_state_.ready; }

    bool canAcceptPacket(const Packet& packet) const {
        return packet.getTypeID() == p_state_.accepted_type_id;
    }

    // TickTock系统
    uint64_t getTickTock() const { return p_state_.tick_tock; }
    
    virtual void tick() {onTick(); p_state_.tick_tock++;}
    
    virtual void tock() {onTock(); p_state_.tick_tock++;}

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
    virtual void serializeImpl([[maybe_unused]] nlohmann::json& j) const {}
    virtual void deserializeImpl([[maybe_unused]] const nlohmann::json& j) {}
    virtual void onTick() {}
    virtual void onTock() {}

    PersistentState p_state_;
};

/**
 * @brief 输入端口类，用于接收数据包
 */
class InputPort : public Port {
public:
    using Port::Port;

    bool receivePacket(std::shared_ptr<Packet> packet) {
        if (!packet || !canAcceptPacket(*packet) || !hasCapacity()) {
            return false;
        }
        p_state_.packets.push_back(packet);
        return true;
    }

    std::shared_ptr<Packet> popPacket() {
        if (!p_state_.valid || p_state_.packets.empty()) {
            return nullptr;
        }
        auto packet = p_state_.packets.front();
        p_state_.packets.erase(p_state_.packets.begin());
        return packet;
    }

    std::shared_ptr<Packet> peekPacket() const {
        return p_state_.packets.empty() ? nullptr : p_state_.packets.front();
    }

protected:
    void onTick() override {
        p_state_.valid = !p_state_.packets.empty();
        p_state_.ready = hasCapacity();
    }
};

/**
 * @brief 输出端口类，用于发送数据包
 */
class OutputPort : public Port {
public:
    OutputPort(const std::string& name, TypeID accepted_type_id, size_t capacity = 0)
        : Port(name, accepted_type_id, capacity) {}

    void connectTo(std::shared_ptr<InputPort> input_port) {
        connected_ports_.push_back(input_port);
    }

    void disconnectFrom(std::shared_ptr<InputPort> input_port) {
        auto it = std::find(connected_ports_.begin(), connected_ports_.end(), input_port);
        if (it != connected_ports_.end()) {
            connected_ports_.erase(it);
        }
    }

    bool sendPacket(std::shared_ptr<Packet> packet) {
        if (!p_state_.valid || !packet || !canAcceptPacket(*packet)) {
            return false;
        }

        bool sent = false;
        for (auto& input_port : connected_ports_) {
            if (input_port && input_port->isReady()) {
                input_port->receivePacket(packet);
                sent = true;
            }
        }
        return sent;
    }

    bool areAllInputPortsFull() const {
        for (const auto& input_port : connected_ports_) {
            if (input_port && input_port->isReady()) {
                return false;
            }
        }
        return true;
    }

    const std::vector<std::shared_ptr<InputPort>>& getConnectedPorts() const {
        return connected_ports_;
    }

protected:
    void onTick() override {
        p_state_.valid = hasCapacity();
        p_state_.ready = !areAllInputPortsFull();
    }

private:
    std::vector<std::shared_ptr<InputPort>> connected_ports_;
};

} // namespace sim 
} // namespace sim 