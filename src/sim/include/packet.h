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

    // 序列化接口
    virtual nlohmann::json toJson() const {
        nlohmann::json j;
        j["type_id"] = getTypeID();
        j["packet_id"] = packet_id_;
        j["src_node_id"] = src_node_id_;
        return j;
    }

    // 反序列化接口
    virtual void fromJson(const nlohmann::json& j) {
        packet_id_ = j["packet_id"];
        src_node_id_ = j["src_node_id"];
    }

    // 获取payload
    virtual std::vector<PayloadID> getPayloads() const { return {}; }

    Packet(NodeID src_node_id = 0) : src_node_id_(src_node_id) {
        packet_id_ = generatePacketID();
    }

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

    nlohmann::json toJson() const override {
        auto j = Packet::toJson();
        j["info"] = info_;
        return j;
    }

    void fromJson(const nlohmann::json& j) override {
        Packet::fromJson(j);
        info_ = j["info"];
    }

    std::string info_;
};

} // namespace sim 