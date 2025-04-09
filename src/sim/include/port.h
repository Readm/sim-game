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

// Port基类
class Port {
public:
    Port(const std::string& name, TypeID accepted_type_id, size_t capacity = 0)
        : name_(name), accepted_type_id_(accepted_type_id), capacity_(capacity) {}
    virtual ~Port() = default;

    // 基本属性
    const std::string& getName() const { return name_; }
    TypeID getAcceptedTypeID() const { return accepted_type_id_; }
    size_t getCapacity() const { return capacity_; }
    bool hasCapacity() const { return capacity_ == 0 || packets_.size() < capacity_; }
    size_t size() const { return packets_.size(); }

    // 包验证
    bool canAcceptPacket(const Packet& packet) const {
        return packet.getTypeID() == accepted_type_id_;
    }

    // 序列化接口
    virtual std::string serialize(SerializationMethod method = SerializationMethod::JSON) const final {
        switch (method) {
            case SerializationMethod::JSON: {
                nlohmann::json j;
                j["name"] = name_;
                j["accepted_type_id"] = accepted_type_id_;
                j["capacity"] = capacity_;
                
                nlohmann::json packets_json;
                for (const auto& packet : packets_) {
                    packets_json.push_back(nlohmann::json::parse(packet->serialize()));
                }
                j["packets"] = packets_json;
                
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
                name_ = j["name"];
                accepted_type_id_ = j["accepted_type_id"];
                capacity_ = j["capacity"];
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

protected:
    // 子类可以重写这些方法来添加自己的序列化逻辑
    virtual void serializeImpl(nlohmann::json& j) const {}
    virtual void deserializeImpl(const nlohmann::json& j) {}

    std::string name_;
    TypeID accepted_type_id_;
    size_t capacity_;
    std::vector<std::shared_ptr<Packet>> packets_;
};

// 输入端口
class InputPort : public Port {
public:
    using Port::Port;

    // 接收数据包
    bool receivePacket(std::shared_ptr<Packet> packet) {
        if (!packet || !canAcceptPacket(*packet) || !hasCapacity()) {
            return false;
        }
        packets_.push_back(packet);
        return true;
    }

    // 获取并移除第一个数据包
    std::shared_ptr<Packet> popPacket() {
        if (packets_.empty()) {
            return nullptr;
        }
        auto packet = packets_.front();
        packets_.erase(packets_.begin());
        return packet;
    }

    // 查看第一个数据包但不移除
    std::shared_ptr<Packet> peekPacket() const {
        return packets_.empty() ? nullptr : packets_.front();
    }
};

// 输出端口
class OutputPort : public Port {
public:
    OutputPort(const std::string& name, TypeID accepted_type_id, size_t capacity = 0)
        : Port(name, accepted_type_id, capacity) {}

    // 连接到输入端口
    void connectTo(std::shared_ptr<InputPort> input_port) {
        connected_ports_.push_back(input_port);
    }

    // 断开与输入端口的连接
    void disconnectFrom(std::shared_ptr<InputPort> input_port) {
        auto it = std::find(connected_ports_.begin(), connected_ports_.end(), input_port);
        if (it != connected_ports_.end()) {
            connected_ports_.erase(it);
        }
    }

    // 发送数据包到所有连接的输入端口
    bool sendPacket(std::shared_ptr<Packet> packet) {
        if (!packet || !canAcceptPacket(*packet)) {
            return false;
        }

        bool sent = false;
        for (auto& input_port : connected_ports_) {
            if (input_port && input_port->hasCapacity()) {
                input_port->receivePacket(packet);
                sent = true;
            }
        }
        return sent;
    }

    // 检查是否所有连接的输入端口都已满
    bool areAllInputPortsFull() const {
        for (const auto& input_port : connected_ports_) {
            if (input_port && input_port->hasCapacity()) {
                return false;
            }
        }
        return true;
    }

    // 获取连接的输入端口
    const std::vector<std::shared_ptr<InputPort>>& getConnectedPorts() const {
        return connected_ports_;
    }

private:
    std::vector<std::shared_ptr<InputPort>> connected_ports_;
};

} // namespace sim 