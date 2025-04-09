#pragma once

#include "node.h"

namespace sim {

// Network节点实现
class Network : public Node {
public:
    REGISTER_TYPE(Network);

    Network() : Node(0) {} // Network的ID固定为0
    TypeID getTypeID() const override { return type_id; }

protected:
    void onTick() override {
        // Network节点在tick时不需要特殊操作
    }

    void onTock() override {
        // Network节点在tock时不需要特殊操作
    }

private:
    std::shared_ptr<Node> createNodeFromJson(const nlohmann::json& j) override {
        TypeID type_id = j["type_id"];
        // TODO: 使用NodeFactory创建节点
        return nullptr;
    }

    std::shared_ptr<Packet> createPacketFromJson(const nlohmann::json& j) override {
        TypeID type_id = j["type_id"];
        // TODO: 使用PacketFactory创建Packet
        return nullptr;
    }
};

} // namespace sim 