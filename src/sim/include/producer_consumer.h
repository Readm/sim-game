#pragma once

#include "node.h"
#include "packet.h"
#include "network.h"
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

namespace sim {

/**
 * @brief Producer node class
 * 
 * The producer node generates a VoidPacket in each Tick-Tock cycle
 * and sends it to the output port
 */
class ProducerNode : public Node {
public:
    REGISTER_TYPE(ProducerNode);
    
    /**
     * @brief Constructor
     * @param id Node ID
     * @param name Node name
     */
    ProducerNode(NodeID id, const std::string& name = "producer")
        : Node(id, VoidPacket::type_id), name_(name) {
        // Add an output port
        addOutputPort("out", VoidPacket::type_id, 5);
    }
    
    /**
     * @brief Get node type ID
     * @return Type ID
     */
    TypeID getTypeID() const override { return type_id; }
    
    /**
     * @brief Get node name
     * @return Node name
     */
    const std::string& getName() const { return name_; }
    
    /**
     * @brief Get count of produced packets
     * @return Packet count
     */
    size_t getProducedCount() const { return produced_count_; }

protected:
    /**
     * @brief Tick phase operation
     * 
     * Check if output port is ready to receive data
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
     * @brief Tock phase operation
     * 
     * Generate packet and send to output port
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
     * @brief Serialization implementation
     * @param j JSON object
     */
    void serializeImpl(nlohmann::json& j) const override {
        j["name"] = name_;
        j["produced_count"] = produced_count_;
    }
    
    /**
     * @brief Deserialization implementation
     * @param j JSON object
     */
    void deserializeImpl(const nlohmann::json& j) override {
        name_ = j["name"];
        produced_count_ = j["produced_count"];
    }

private:
    /**
     * @brief Create node from JSON
     * @param j JSON object
     * @return Node pointer
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
     * @brief Create packet from JSON
     * @param j JSON object
     * @return Packet pointer
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
 * @brief Consumer node class
 * 
 * Consumer node reads packets from input port in each Tick-Tock cycle
 */
class ConsumerNode : public Node {
public:
    REGISTER_TYPE(ConsumerNode);
    
    /**
     * @brief Constructor
     * @param id Node ID
     * @param name Node name
     */
    ConsumerNode(NodeID id, const std::string& name = "consumer")
        : Node(id, VoidPacket::type_id), name_(name) {
        // Add an input port
        addInputPort("in", VoidPacket::type_id, 5);
    }
    
    /**
     * @brief Get node type ID
     * @return Type ID
     */
    TypeID getTypeID() const override { return type_id; }
    
    /**
     * @brief Get node name
     * @return Node name
     */
    const std::string& getName() const { return name_; }
    
    /**
     * @brief Get count of consumed packets
     * @return Packet count
     */
    size_t getConsumedCount() const { return consumed_count_; }

protected:
    /**
     * @brief Tick phase operation
     * 
     * Check if input port has available data
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
     * @brief Tock phase operation
     * 
     * Read packet from input port
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
     * @brief Serialization implementation
     * @param j JSON object
     */
    void serializeImpl(nlohmann::json& j) const override {
        j["name"] = name_;
        j["consumed_count"] = consumed_count_;
    }
    
    /**
     * @brief Deserialization implementation
     * @param j JSON object
     */
    void deserializeImpl(const nlohmann::json& j) override {
        name_ = j["name"];
        consumed_count_ = j["consumed_count"];
    }

private:
    /**
     * @brief Create node from JSON
     * @param j JSON object
     * @return Node pointer
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
     * @brief Create packet from JSON
     * @param j JSON object
     * @return Packet pointer
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