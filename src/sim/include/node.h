#pragma once

#include "type.h"
#include "packet.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace sim {

class Node {
public:
    Node(NodeID id = 0) : node_id_(id) {}
    virtual ~Node() = default;

    // 获取Node的TypeID
    virtual TypeID getTypeID() const = 0;

    // 获取Node的ID
    NodeID getNodeID() const { return node_id_; }

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

    // 序列化接口
    virtual nlohmann::json toJson() const {
        nlohmann::json j;
        j["type_id"] = getTypeID();
        j["node_id"] = node_id_;
        
        // 序列化子节点
        nlohmann::json children_json;
        for (const auto& child : children_) {
            children_json.push_back(child->toJson());
        }
        j["children"] = children_json;

        // 序列化缓冲区
        nlohmann::json buffer_json;
        for (const auto& packet : buffer_) {
            buffer_json.push_back(packet->toJson());
        }
        j["buffer"] = buffer_json;

        return j;
    }

    // 反序列化接口
    virtual void fromJson(const nlohmann::json& j) {
        node_id_ = j["node_id"];
        
        // 反序列化子节点
        children_.clear();
        for (const auto& child_json : j["children"]) {
            // 注意：这里需要根据type_id创建正确的子节点类型
            // 实际实现中需要注册工厂函数
            auto child = createNodeFromJson(child_json);
            if (child) {
                children_.push_back(child);
            }
        }

        // 反序列化缓冲区
        buffer_.clear();
        for (const auto& packet_json : j["buffer"]) {
            // 注意：这里需要根据type_id创建正确的Packet类型
            // 实际实现中需要注册工厂函数
            auto packet = createPacketFromJson(packet_json);
            if (packet) {
                buffer_.push_back(packet);
            }
        }
    }

protected:
    NodeID node_id_;
    std::vector<std::shared_ptr<Node>> children_;
    std::vector<std::shared_ptr<Packet>> buffer_;

private:
    // 这些函数需要在具体实现中提供
    virtual std::shared_ptr<Node> createNodeFromJson(const nlohmann::json& j) = 0;
    virtual std::shared_ptr<Packet> createPacketFromJson(const nlohmann::json& j) = 0;
};

// Network实现
class Network : public Node {
public:
    REGISTER_TYPE(Network);

    Network() : Node(0) {} // Network的ID固定为0

    TypeID getTypeID() const override { return type_id; }

private:
    std::shared_ptr<Node> createNodeFromJson(const nlohmann::json& j) override {
        // 实际实现中需要根据type_id创建正确的节点类型
        return nullptr;
    }

    std::shared_ptr<Packet> createPacketFromJson(const nlohmann::json& j) override {
        // 实际实现中需要根据type_id创建正确的Packet类型
        return nullptr;
    }
};

} // namespace sim 