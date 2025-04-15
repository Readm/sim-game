/**
 * @file packet.h
 * @brief 定义了仿真系统中的数据包类型
 * 
 * 该文件实现了数据包系统的基础设施：
 * - Packet基类：提供基本的数据包功能
 * - VoidPacket：空数据包，用于测试和特殊场景
 * - InfoPacket：携带字符串信息的数据包
 * 
 * 数据包特性：
 * - 支持类型安全的数据传输
 * - 提供源节点追踪
 * - 实现了全局唯一的数据包ID
 * - 支持序列化和反序列化
 * - 可扩展的负载系统
 */

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


namespace sim {

/**
 * @brief 数据包基类，提供数据传输的基本功能
 * 
 * Packet类实现了以下功能：
 * - 类型安全的数据传输
 * - 源节点追踪
 * - 全局唯一的数据包ID
 * - 序列化/反序列化支持
 * - 可扩展的负载系统
 */
struct Packet {
    virtual ~Packet() = default;

    /**
     * @brief 获取数据包的类型ID
     * @return 数据包类型ID
     */
    virtual TypeID getTypeID() const = 0;

    /**
     * @brief 获取数据包的全局唯一ID
     * @return 数据包ID
     */
    PacketID getPacketID() const { return packet_id_; }

    /**
     * @brief 获取源节点ID
     * @return 生成该数据包的节点ID
     */
    NodeID getSrcNodeID() const { return src_node_id_; }

    /**
     * @brief 获取数据包的负载ID列表
     * @return 负载ID的向量
     */
    virtual std::vector<PayloadID> getPayloads() const { return {}; }

    /**
     * @brief 构造一个新的数据包
     * @param src_node_id 源节点ID
     * @param id 数据包ID
     */
    Packet(NodeID src_node_id = 0, PacketID id = PacketID())
        : src_node_id_(src_node_id), packet_id_(id) {}

    // 序列化接口
    virtual std::string serialize(SerializationMethod method = SerializationMethod::JSON) const final {
        switch (method) {
            case SerializationMethod::JSON: {
                nlohmann::json j;
                j["type_id"] = getTypeID();
                j["packet_id"] = {
                    {"node_id", packet_id_.node_id},
                    {"local_seq", packet_id_.local_seq}
                };
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
                packet_id_ = PacketID(
                    j["packet_id"]["node_id"],
                    j["packet_id"]["local_seq"]
                );
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

    virtual void serializeImpl(nlohmann::json& j) const {}
    virtual void deserializeImpl(const nlohmann::json& j) {}

    NodeID src_node_id_;
    PacketID packet_id_;
};

/**
 * @brief 空数据包类，用于测试和特殊场景
 * 
 * VoidPacket不携带任何数据，主要用于：
 * - 系统测试
 * - 心跳包
 * - 同步信号
 */
struct VoidPacket : public Packet {
    REGISTER_TYPE(VoidPacket);
    
    /**
     * @brief 构造一个新的空数据包
     * @param src_node_id 源节点ID
     * @param id 数据包ID
     */
    VoidPacket(NodeID src_node_id = 0, PacketID id = PacketID()) 
        : Packet(src_node_id, id) {}
    
    TypeID getTypeID() const override { return type_id; }
};

/**
 * @brief 信息数据包类，用于传输字符串信息
 * 
 * InfoPacket携带一个字符串信息，主要用于：
 * - 调试信息传输
 * - 日志记录
 * - 状态报告
 */
struct InfoPacket : public Packet {
    REGISTER_TYPE(InfoPacket);

    /**
     * @brief 构造一个新的信息数据包
     * @param src_node_id 源节点ID
     * @param id 数据包ID
     * @param info 要传输的信息字符串
     */
    InfoPacket(NodeID src_node_id = 0, PacketID id = PacketID(), const std::string& info = "")
        : Packet(src_node_id, id), info_(info) {}

    TypeID getTypeID() const override { return type_id; }

    /**
     * @brief 获取信息内容
     * @return 信息字符串的常引用
     */
    const std::string& getInfo() const { return info_; }

    /**
     * @brief 设置信息内容
     * @param info 新的信息字符串
     */
    void setInfo(const std::string& info) { info_ = info; }

    void serializeImpl(nlohmann::json& j) const override {
        j["info"] = info_;
    }

    void deserializeImpl(const nlohmann::json& j) override {
        info_ = j["info"];
    }

    std::string info_;
};

} // namespace sim 