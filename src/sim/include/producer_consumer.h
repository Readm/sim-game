/**
 * @file producer_consumer.h
 * @brief 定义了仿真系统中的生产者和消费者节点
 * 
 * 该文件实现了经典的生产者-消费者模式节点：
 * - ProducerNode：生产者节点，定期生成数据包
 * - ConsumerNode：消费者节点，消费接收到的数据包
 * 
 * 这些节点是仿真系统的基本示例，展示了：
 * - 如何实现自定义节点类型
 * - Tick-Tock时序机制的使用
 * - 端口系统的连接和数据传输
 * - 节点状态的序列化和反序列化
 * - 数据包的生成和处理流程
 * 
 * 生产者-消费者模式是验证仿真系统功能的理想测试用例。
 */

#pragma once

#include "node.h"
#include "packet.h"
#include "network.h"
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

namespace sim {

/**
 * @brief 生产者节点类
 * 
 * 生产者节点在每个Tick-Tock周期中生成一个VoidPacket
 * 并将其发送到输出端口
 */
class ProducerNode : public Node {
public:
    REGISTER_TYPE(ProducerNode);
    
    /**
     * @brief 构造函数
     * @param id 节点ID
     * @param name 节点名称
     */
    ProducerNode(NodeID id, const std::string& name = "producer")
        : Node(id, VoidPacket::type_id), name_(name) {
        // Add an output port
        addOutputPort("out", VoidPacket::type_id, 5);
    }
    
    /**
     * @brief 获取节点类型ID
     * @return 类型ID
     */
    TypeID getTypeID() const override { return type_id; }
    
    /**
     * @brief 获取节点名称
     * @return 节点名称
     */
    const std::string& getName() const { return name_; }
    
    /**
     * @brief 获取已生产的数据包数量
     * @return 数据包数量
     */
    size_t getProducedCount() const { return produced_count_; }

protected:
    /**
     * @brief Tick阶段操作
     * 
     * 检查输出端口是否准备好接收数据
     */
    void onTick() override {
        // Check if output port is ready to receive data during tick phase
        auto out_port = getOutputPort("out");
        if (out_port && !out_port->areAllInputPortsFull()) {
            should_produce_ = true;
        } else {
            should_produce_ = false;
        }
    }
    
    /**
     * @brief Tock阶段操作
     * 
     * 生成数据包并发送到输出端口
     */
    void onTock() override {
        if (should_produce_) {
            auto out_port = getOutputPort("out");
            if (out_port) {
                auto packet = spawnPacket<VoidPacket>();
                if (packet && out_port->sendPacket(packet)) {
                    produced_count_++;
                    SIM_INFO("Producer[" + name_ + "] generated a packet, total: " + std::to_string(produced_count_));
                }
            }
        }
    }
    
    /**
     * @brief 序列化实现
     * @param j JSON对象
     */
    void serializeImpl(nlohmann::json& j) const override {
        j["name"] = name_;
        j["produced_count"] = produced_count_;
    }
    
    /**
     * @brief 反序列化实现
     * @param j JSON对象
     */
    void deserializeImpl(const nlohmann::json& j) override {
        name_ = j["name"];
        produced_count_ = j["produced_count"];
    }

private:
    /**
     * @brief 从JSON创建节点
     * @param j JSON对象
     * @return 节点指针
     */
    std::shared_ptr<Node> createNodeFromJson(const nlohmann::json& j) override {
        TypeID type_id = j["type_id"];
        if (type_id == ProducerNode::type_id) {
            NodeID id = j["node_id"];
            std::string name = j.contains("name") ? j["name"] : "producer";
            return std::make_shared<ProducerNode>(id, name);
        }
        return nullptr;
    }
    
    /**
     * @brief 从JSON创建数据包
     * @param j JSON对象
     * @return 数据包指针
     */
    std::shared_ptr<Packet> createPacketFromJson(const nlohmann::json& j) override {
        TypeID type_id = j["type_id"];
        if (type_id == VoidPacket::type_id) {
            NodeID src_node_id = j["src_node_id"];
            PacketID packet_id(j["packet_id"]["node_id"], j["packet_id"]["local_seq"]);
            return std::make_shared<VoidPacket>(src_node_id, packet_id);
        }
        return nullptr;
    }

    std::string name_;
    size_t produced_count_ = 0;
    bool should_produce_ = false;
};

/**
 * @brief 消费者节点类
 * 
 * 消费者节点在每个Tick-Tock周期中从输入端口读取数据包
 */
class ConsumerNode : public Node {
public:
    REGISTER_TYPE(ConsumerNode);
    
    /**
     * @brief 构造函数
     * @param id 节点ID
     * @param name 节点名称
     */
    ConsumerNode(NodeID id, const std::string& name = "consumer")
        : Node(id, VoidPacket::type_id), name_(name) {
        // Add an input port
        addInputPort("in", VoidPacket::type_id, 5);
    }
    
    /**
     * @brief 获取节点类型ID
     * @return 类型ID
     */
    TypeID getTypeID() const override { return type_id; }
    
    /**
     * @brief 获取节点名称
     * @return 节点名称
     */
    const std::string& getName() const { return name_; }
    
    /**
     * @brief 获取已消费的数据包数量
     * @return 数据包数量
     */
    size_t getConsumedCount() const { return consumed_count_; }

protected:
    /**
     * @brief Tick阶段操作
     * 
     * 检查输入端口是否有可用数据
     */
    void onTick() override {
        // Check if input port has available data during tick phase
        auto in_port = getInputPort("in");
        if (in_port && in_port->isValid()) {
            can_consume_ = true;
        } else {
            can_consume_ = false;
        }
    }
    
    /**
     * @brief Tock阶段操作
     * 
     * 从输入端口读取数据包
     */
    void onTock() override {
        if (can_consume_) {
            auto in_port = getInputPort("in");
            if (in_port) {
                auto packet = in_port->popPacket();
                if (packet) {
                    consumed_count_++;
                    SIM_INFO("Consumer[" + name_ + "] consumed a packet, total: " + std::to_string(consumed_count_));
                }
            }
        }
    }
    
    /**
     * @brief 序列化实现
     * @param j JSON对象
     */
    void serializeImpl(nlohmann::json& j) const override {
        j["name"] = name_;
        j["consumed_count"] = consumed_count_;
    }
    
    /**
     * @brief 反序列化实现
     * @param j JSON对象
     */
    void deserializeImpl(const nlohmann::json& j) override {
        name_ = j["name"];
        consumed_count_ = j["consumed_count"];
    }

private:
    /**
     * @brief 从JSON创建节点
     * @param j JSON对象
     * @return 节点指针
     */
    std::shared_ptr<Node> createNodeFromJson(const nlohmann::json& j) override {
        TypeID type_id = j["type_id"];
        if (type_id == ConsumerNode::type_id) {
            NodeID id = j["node_id"];
            std::string name = j.contains("name") ? j["name"] : "consumer";
            return std::make_shared<ConsumerNode>(id, name);
        }
        return nullptr;
    }
    
    /**
     * @brief 从JSON创建数据包
     * @param j JSON对象
     * @return 数据包指针
     */
    std::shared_ptr<Packet> createPacketFromJson(const nlohmann::json& j) override {
        TypeID type_id = j["type_id"];
        if (type_id == VoidPacket::type_id) {
            NodeID src_node_id = j["src_node_id"];
            PacketID packet_id(j["packet_id"]["node_id"], j["packet_id"]["local_seq"]);
            return std::make_shared<VoidPacket>(src_node_id, packet_id);
        }
        return nullptr;
    }

    std::string name_;
    size_t consumed_count_ = 0;
    bool can_consume_ = false;
};

} // namespace sim 