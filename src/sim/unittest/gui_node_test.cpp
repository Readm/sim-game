#include "doctest/doctest.h"
#include "common.h"
#include "node.h"
#include "packet.h"
#include "network.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace sim;

// 自定义多输入节点
class MultiInputNode : public Node {
public:
    static constexpr TypeID type_id = generateTypeID("MultiInputNode");
    inline static bool type_registered = TypeRegistry::getInstance().registerType(type_id, "MultiInputNode");

    MultiInputNode(NodeID id = 0) : Node(id, VoidPacket::type_id) {
        // 创建两个输入端口和一个输出端口
        addInputPort("input1", VoidPacket::type_id, 2);
        addInputPort("input2", VoidPacket::type_id, 2);
        addOutputPort("output", VoidPacket::type_id);
    }

    TypeID getTypeID() const override { return type_id; }
    void tick() override {}
    void tock() override {}

private:
    std::shared_ptr<Node> createNodeFromJson(const nlohmann::json& json) override {
        auto node = std::make_shared<MultiInputNode>();
        node->deserialize(json.dump());
        return node;
    }
    std::shared_ptr<Packet> createPacketFromJson(const nlohmann::json& json) override {
        auto packet = std::make_shared<VoidPacket>();
        packet->deserialize(json.dump());
        return packet;
    }
};

// 自定义多输出节点
class MultiOutputNode : public Node {
public:
    static constexpr TypeID type_id = generateTypeID("MultiOutputNode");
    inline static bool type_registered = TypeRegistry::getInstance().registerType(type_id, "MultiOutputNode");

    MultiOutputNode(NodeID id = 0) : Node(id, VoidPacket::type_id) {
        // 创建一个输入端口和两个输出端口
        addInputPort("input", VoidPacket::type_id, 2);
        addOutputPort("output1", VoidPacket::type_id);
        addOutputPort("output2", VoidPacket::type_id);
    }

    TypeID getTypeID() const override { return type_id; }
    void tick() override {}
    void tock() override {}

private:
    std::shared_ptr<Node> createNodeFromJson(const nlohmann::json& json) override {
        auto node = std::make_shared<MultiOutputNode>();
        node->deserialize(json.dump());
        return node;
    }
    std::shared_ptr<Packet> createPacketFromJson(const nlohmann::json& json) override {
        auto packet = std::make_shared<VoidPacket>();
        packet->deserialize(json.dump());
        return packet;
    }
};

TEST_CASE("Generate GUI test JSON") {
    // 创建网络
    auto network = std::make_shared<Network>();
    
    // 创建节点
    auto multiInput = std::make_shared<MultiInputNode>(1);
    auto multiOutput = std::make_shared<MultiOutputNode>(2);
    
    // 添加节点到网络
    network->addChild(multiInput);
    network->addChild(multiOutput);
    
    // 连接节点
    auto out_port = multiInput->getOutputPort("output");
    auto in_port = multiOutput->getInputPort("input");
    REQUIRE(out_port != nullptr);
    REQUIRE(in_port != nullptr);
    out_port->connectTo(in_port);
    
    // 序列化为JSON
    std::string network_str = network->serialize();
    json network_json = json::parse(network_str);
    
    // 保存到文件
    std::ofstream file("gui_node_test.json");
    file << network_json.dump(4);
    file.close();
    
    // 验证JSON文件是否包含必要的字段
    CHECK(network_json.contains("type_id"));
    CHECK(network_json.contains("node_id"));
    CHECK(network_json.contains("children"));
    
    // 验证节点数量
    CHECK(network_json["children"].size() == 2);
    
    // 验证节点类型
    CHECK(network_json["children"][0]["type_id"] == MultiInputNode::type_id);
    CHECK(network_json["children"][1]["type_id"] == MultiOutputNode::type_id);
} 