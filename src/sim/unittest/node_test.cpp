#include "doctest/doctest.h"
#include "sim/include/common.h"
#include "sim/include/node.h"
#include "sim/include/packet.h"
#include <chrono>
#include <thread>
#include <sstream>

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

TEST_CASE("Node Simulation") {
    using namespace sim;
    
    SUBCASE("最快模式测试") {
        auto node = std::make_shared<TestNode>(1);
        std::stringstream out;
        
        // 模拟10个时间单位
        node->simulate(SimulationMode::FASTEST, 10, std::cin, out);
        
        // 检查输出流中只有一次序列化结果
        std::string line;
        int line_count = 0;
        std::stringstream ss(out.str());
        while (std::getline(ss, line)) {
            ++line_count;
            auto json = nlohmann::json::parse(line);
            CHECK(json["tick_tock"] == 10);  // 最后状态应该是10
        }
        CHECK(line_count == 1);  // 应该只有一行输出
    }
    
    SUBCASE("跟踪模式测试") {
        auto node = std::make_shared<TestNode>(1);
        std::stringstream out;
        
        // 模拟5个时间单位
        node->simulate(SimulationMode::TRACE, 5, std::cin, out);
        
        // 检查输出流中有5次序列化结果
        std::string line;
        int line_count = 0;
        std::stringstream ss(out.str());
        while (std::getline(ss, line)) {
            ++line_count;
            auto json = nlohmann::json::parse(line);
            CHECK(json["tick_tock"] == line_count);  // 每行的tick_tock应该递增
        }
        CHECK(line_count == 5);  // 应该有5行输出
    }
    
    SUBCASE("单步模式测试") {
        auto node = std::make_shared<TestNode>(1);
        std::stringstream in, out;
        
        // 准备输入流（每次需要按回车）
        in << "\n\n\n\n";  // 2个时间单位需要4个回车（每个时间单位的tick和tock各需要一个）
        
        // 模拟2个时间单位
        node->simulate(SimulationMode::STEP, 2, in, out);
        
        // 检查输出流
        std::string line;
        int state_count = 0;
        int prompt_count = 0;
        std::stringstream ss(out.str());
        while (std::getline(ss, line)) {
            if (line.find("Before") != std::string::npos) {
                ++prompt_count;  // 统计提示信息的数量
            } else if (line.find("Final") != std::string::npos) {
                continue;  // 跳过最终状态的提示
            } else {
                ++state_count;  // 统计状态输出的数量
                if (!line.empty()) {
                    auto json = nlohmann::json::parse(line);
                    CHECK(json["tick_tock"] <= 2);  // tick_tock不应超过2
                }
            }
        }
        CHECK(prompt_count == 4);  // 应该有4次提示（2个时间单位，每个tick和tock各一次）
        CHECK(state_count == 5);   // 应该有5次状态输出（4次中间状态+1次最终状态）
    }
    
    SUBCASE("子节点模拟测试") {
        auto parent = std::make_shared<TestNode>(1);
        auto child = std::make_shared<TestNode>(2);
        parent->addChild(child);
        std::stringstream out;
        
        // 模拟3个时间单位
        parent->simulate(SimulationMode::FASTEST, 3, std::cin, out);
        
        // 检查最终状态
        std::string result = out.str();
        auto json = nlohmann::json::parse(result);
        CHECK(json["tick_tock"] == 3);
        CHECK(json["children"][0]["tick_tock"] == 3);  // 子节点也应该模拟了3个时间单位
    }
} 