#include "doctest/doctest.h"
#include "../include/node.h"
#include "../include/packet.h"

// 测试用的Node类型
class TestNode : public sim::Node {
public:
    static constexpr sim::TypeID type_id = sim::generateTypeID("TestNode");
    inline static bool type_registered = sim::TypeRegistry::getInstance().registerType(type_id, "TestNode");

    using Node::Node;  // 继承基类的构造函数
    sim::TypeID getTypeID() const override { return type_id; }
    
private:
    std::shared_ptr<sim::Node> createNodeFromJson(const nlohmann::json&) override {
        return nullptr;
    }
    std::shared_ptr<sim::Packet> createPacketFromJson(const nlohmann::json&) override {
        return nullptr;
    }
};

TEST_CASE("Node Base Class") {
    using namespace sim;

    TestNode node(123);
    CHECK(node.getNodeID() == 123);
    CHECK(node.getTypeID() == TestNode::type_id);
    CHECK(node.getChildren().empty());
    CHECK(node.getBuffer().empty());
}

TEST_CASE("Node Children Management") {
    using namespace sim;

    auto parent = std::make_shared<TestNode>(1);
    auto child1 = std::make_shared<TestNode>(2);
    auto child2 = std::make_shared<TestNode>(3);

    parent->addChild(child1);
    parent->addChild(child2);

    const auto& children = parent->getChildren();
    CHECK(children.size() == 2);
    CHECK(children[0]->getNodeID() == 2);
    CHECK(children[1]->getNodeID() == 3);
}

TEST_CASE("Node Buffer Management") {
    using namespace sim;

    auto node = std::make_shared<TestNode>(1);
    auto packet1 = std::make_shared<InfoPacket>(1, "Packet 1");
    auto packet2 = std::make_shared<InfoPacket>(1, "Packet 2");

    node->addPacket(packet1);
    node->addPacket(packet2);

    const auto& buffer = node->getBuffer();
    CHECK(buffer.size() == 2);
    CHECK(buffer[0]->getTypeID() == InfoPacket::type_id);
    CHECK(buffer[1]->getTypeID() == InfoPacket::type_id);

    node->clearBuffer();
    CHECK(node->getBuffer().empty());
}

TEST_CASE("Network Class") {
    using namespace sim;

    Network network;
    CHECK(network.getNodeID() == 0);
    CHECK(network.getTypeID() == Network::type_id);
    CHECK(network.getChildren().empty());
    CHECK(network.getBuffer().empty());
}

TEST_CASE("Node Serialization") {
    using namespace sim;

    auto node = std::make_shared<TestNode>(1);
    auto child = std::make_shared<TestNode>(2);
    auto packet = std::make_shared<InfoPacket>(1, "Test Packet");

    node->addChild(child);
    node->addPacket(packet);

    auto json = node->toJson();
    CHECK(json["type_id"] == TestNode::type_id);
    CHECK(json["node_id"] == 1);
    CHECK(json["children"].size() == 1);
    CHECK(json["buffer"].size() == 1);
    CHECK(json["children"][0]["node_id"] == 2);
    CHECK(json["buffer"][0]["type_id"] == InfoPacket::type_id);
} 