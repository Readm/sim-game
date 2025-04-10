#include "doctest/doctest.h"
#include "../include/common.h"
#include "../include/node.h"
#include "../include/packet.h"
#include <chrono>
#include <thread>

namespace sim {

// 测试用的Node类
class TestNode : public Node {
public:
    static constexpr TypeID type_id = generateTypeID("TestNode");
    inline static bool type_registered = TypeRegistry::getInstance().registerType(type_id, "TestNode");

    TestNode(NodeID id = 0) : Node(id, VoidPacket::type_id) {} // TestNode 只能生成 VoidPacket
    TypeID getTypeID() const override { return type_id; }
    
protected:
    void onTick() override {
        // 模拟耗时操作
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void onTock() override {
        // 模拟耗时操作
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
private:
    std::shared_ptr<Node> createNodeFromJson(const nlohmann::json& json) override {
        auto node = std::make_shared<TestNode>();
        node->deserialize(json.dump());
        return node;
    }
    std::shared_ptr<Packet> createPacketFromJson(const nlohmann::json& json) override {
        auto packet = std::make_shared<VoidPacket>();
        packet->deserialize(json.dump());
        return packet;
    }
};

} // namespace sim

TEST_CASE("Node Base Class") {
    using namespace sim;

    TestNode node(123);
    CHECK(node.getNodeID() == 123);
    CHECK(node.getTypeID() == TestNode::type_id);
    CHECK(node.getPacketTypeID() == VoidPacket::type_id);
    CHECK(node.getChildren().empty());
    CHECK(node.getBuffer().empty());
    CHECK(node.getTickTock() == 0);
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

TEST_CASE("Node Port Management") {
    using namespace sim;

    auto node = std::make_shared<TestNode>(1);
    
    // 测试添加新端口
    auto new_in = node->addInputPort("new_in", VoidPacket::type_id, 2);
    auto new_out = node->addOutputPort("new_out", VoidPacket::type_id);
    CHECK(node->getInputPort("new_in") == new_in);
    CHECK(node->getOutputPort("new_out") == new_out);
    CHECK(node->getInputPorts().size() == 1);
    CHECK(node->getOutputPorts().size() == 1);
}

TEST_CASE("Node TickTock System") {
    using namespace sim;

    auto node = std::make_shared<TestNode>(1);
    auto child = std::make_shared<TestNode>(2);
    node->addChild(child);

    // 初始状态检查
    CHECK(node->getTickTock() == 0);
    CHECK(child->getTickTock() == 0);

    // 执行一个tick-tock周期
    node->tick();  // 这会递归调用child的tick
    node->tock();  // 这会递归调用child的tock

    // 检查tick-tock后的状态
    CHECK(node->getTickTock() == 1);
    CHECK(child->getTickTock() == 1);
}

TEST_CASE("Node Serialization") {
    using namespace sim;

    auto node = std::make_shared<TestNode>(1);
    auto child = std::make_shared<TestNode>(2);
    auto packet = std::make_shared<VoidPacket>(1);

    // 添加端口
    auto in_port = node->addInputPort("in", VoidPacket::type_id, 2);
    auto out_port = node->addOutputPort("out", VoidPacket::type_id);

    node->addChild(child);
    in_port->receivePacket(packet);

    // 执行几个tick-tock周期
    for (int i = 0; i < 3; ++i) {
        node->tick();
        node->tock();
    }

    auto json = nlohmann::json::parse(node->serialize());
    CHECK(json["type_id"] == TestNode::type_id);
    CHECK(json["node_id"] == 1);
    CHECK(json["tick_tock"] == 3);
    CHECK(json["children"].size() == 1);
    CHECK(json["input_ports"]["in"] != nullptr);
    CHECK(json["output_ports"]["out"] != nullptr);

    // 创建新节点并反序列化
    auto new_node = std::make_shared<TestNode>();
    new_node->deserialize(json.dump());
    CHECK(new_node->getNodeID() == 1);
    CHECK(new_node->getTickTock() == 3);
    CHECK(new_node->getChildren().size() == 1);
    CHECK(new_node->getInputPort("in") != nullptr);
    CHECK(new_node->getOutputPort("out") != nullptr);
}

TEST_CASE("Node Packet Spawning") {
    using namespace sim;

    TestNode node(123);
    
    // 测试生成正确类型的Packet
    auto void_packet = node.spawnPacket<VoidPacket>();
    CHECK(void_packet != nullptr);
    CHECK(void_packet->getSrcNodeID() == 123);
    CHECK(void_packet->getTypeID() == VoidPacket::type_id);

    // 测试生成错误类型的Packet
    auto info_packet = node.spawnPacket<InfoPacket>();
    CHECK(info_packet == nullptr);
}

TEST_CASE("Node Parallel Execution") {
    using namespace sim;
    using namespace std::chrono;

    auto parent = std::make_shared<TestNode>(1);
    const int num_children = 4;
    
    // 添加子节点
    for (int i = 0; i < num_children; ++i) {
        parent->addChild(std::make_shared<TestNode>(i + 2));
    }

    SUBCASE("Serial Execution") {
        Node::setParallelizationMethod(ParallelizationMethod::NONE);
        
        auto start = high_resolution_clock::now();
        parent->tick();
        parent->tock();
        auto end = high_resolution_clock::now();
        
        auto serial_duration = duration_cast<milliseconds>(end - start).count();
        // 串行执行应该接近 (num_children + 1) * 200ms
        CHECK(serial_duration >= (num_children + 1) * 200);
    }

    SUBCASE("Parallel Execution with Thread Pool") {
        Node::setParallelizationMethod(ParallelizationMethod::THREAD_POOL);
        
        auto start = high_resolution_clock::now();
        parent->tick();
        parent->tock();
        auto end = high_resolution_clock::now();
        
        auto parallel_duration = duration_cast<milliseconds>(end - start).count();
        // 并行执行应该接近 2 * 200ms（父节点的tick和tock各100ms）
        CHECK(parallel_duration >= 400);
        CHECK(parallel_duration < (num_children + 1) * 200);
    }
} 