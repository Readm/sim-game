#pragma once

#include "type.h"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <vector>

namespace sim {

class Packet;
class VoidPacket;
class InfoPacket;
}  // namespace sim

// 前向声明测试类
class TestPacket;

namespace sim {

struct Packet {
    virtual ~Packet() = default;

    // 获取Packet的TypeID
    virtual TypeID getTypeID() const = 0;

    // 获取Packet的ID
    PacketID getPacketID() const { return packet_id_; }
    NodeID getSrcNodeID() const { return src_node_id_; }

    // 获取payload
    virtual std::vector<PayloadID> getPayloads() const { return {}; }

    Packet(NodeID src_node_id = 0) : src_node_id_(src_node_id) {
        packet_id_ = generatePacketID();
    }

    // 序列化接口
    virtual std::string serialize(SerializationMethod method = SerializationMethod::JSON) const final {
        switch (method) {
            case SerializationMethod::JSON: {
                nlohmann::json j;
                j["type_id"] = getTypeID();
                j["packet_id"] = packet_id_;
                j["src_node_id"] = src_node_id_;
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
                packet_id_ = j["packet_id"];
                src_node_id_ = j["src_node_id"];
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

    NodeID src_node_id_;
    PacketID packet_id_;

    static PacketID generatePacketID() {
        static PacketID next_id = 1;
        return next_id++;
    }
};

// VoidPacket实现
struct VoidPacket : public Packet {
    REGISTER_TYPE(VoidPacket);
    using Packet::Packet;  // 继承基类的构造函数
    TypeID getTypeID() const override { return type_id; }
};

// InfoPacket实现
struct InfoPacket : public Packet {
    REGISTER_TYPE(InfoPacket);

    InfoPacket(NodeID src_node_id = 0, const std::string& info = "")
        : Packet(src_node_id), info_(info) {}

    TypeID getTypeID() const override { return type_id; }

    const std::string& getInfo() const { return info_; }
    void setInfo(const std::string& info) { info_ = info; }

protected:
    void serializeImpl(nlohmann::json& j) const override {
        j["info"] = info_;
    }

    void deserializeImpl(const nlohmann::json& j) override {
        info_ = j["info"];
    }

private:
    std::string info_;
};

} // namespace sim 