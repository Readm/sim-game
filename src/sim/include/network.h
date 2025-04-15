/**
 * @file network.h
 * @brief 定义了仿真系统中的网络节点
 * 
 * Network类是一个特殊的节点类型：
 * - 作为整个仿真网络的根节点
 * - 固定ID为0
 * - 只能生成VoidPacket
 * - 提供节点和数据包的工厂方法
 * 
 * 该类主要用于：
 * - 管理整个仿真网络的拓扑结构
 * - 提供节点和数据包的序列化/反序列化支持
 * - 作为仿真系统的顶层容器
 */

#pragma once

#include "node.h"

namespace sim {

/**
 * @brief 网络节点类，作为仿真系统的根节点
 * 
 * Network类是一个特殊的节点类型，具有以下特点：
 * - 固定ID为0
 * - 只能生成VoidPacket
 * - 作为整个仿真网络的根节点
 * - 提供节点和数据包的工厂方法
 */
class Network : public Node {
public:
    REGISTER_TYPE(Network);

    /**
     * @brief 构造一个新的网络节点
     * 
     * 网络节点的ID固定为0，只能生成VoidPacket类型的数据包
     */
    Network() : Node(0, VoidPacket::type_id) {}

    /**
     * @brief 获取网络节点的类型ID
     * @return 网络节点类型ID
     */
    TypeID getTypeID() const override { return type_id; }

protected:
    /**
     * @brief Tick阶段的操作
     * 
     * 网络节点在Tick阶段不需要特殊操作
     */
    void onTick() override {
        // Network节点在tick时不需要特殊操作
    }

    /**
     * @brief Tock阶段的操作
     * 
     * 网络节点在Tock阶段不需要特殊操作
     */
    void onTock() override {
        // Network节点在tock时不需要特殊操作
    }

private:
    /**
     * @brief 从JSON创建节点的工厂方法
     * @param j JSON对象
     * @return 创建的节点的智能指针
     * 
     * TODO: 使用NodeFactory创建节点
     */
    std::shared_ptr<Node> createNodeFromJson(const nlohmann::json& j) override {
        TypeID type_id = j["type_id"];
        // TODO: 使用NodeFactory创建节点
        return nullptr;
    }

    /**
     * @brief 从JSON创建数据包的工厂方法
     * @param j JSON对象
     * @return 创建的数据包的智能指针
     * 
     * TODO: 使用PacketFactory创建Packet
     */
    std::shared_ptr<Packet> createPacketFromJson(const nlohmann::json& j) override {
        TypeID type_id = j["type_id"];
        // TODO: 使用PacketFactory创建Packet
        return nullptr;
    }
};

} // namespace sim 