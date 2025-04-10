#include "doctest/doctest.h"
#include "port.h"
#include "packet.h"
#include "node.h"
#include "nlohmann/json.hpp"

TEST_CASE("Port Base Class") {
    using namespace sim;

    // 创建一个接受VoidPacket的端口
    Port port("test_port", VoidPacket::type_id, 2);
    
    CHECK(port.getName() == "test_port");
    CHECK(port.getAcceptedTypeID() == VoidPacket::type_id);
    CHECK(port.getCapacity() == 2);
    CHECK(port.hasCapacity() == true);
    CHECK(port.size() == 0);
}

TEST_CASE("Input Port") {
    using namespace sim;

    auto input_port = std::make_shared<InputPort>("input", VoidPacket::type_id, 2);
    auto packet1 = std::make_shared<VoidPacket>(1);
    auto packet2 = std::make_shared<VoidPacket>(2);
    auto packet3 = std::make_shared<VoidPacket>(3);
    auto info_packet = std::make_shared<InfoPacket>(4, PacketID(), "test");

    // 测试接收正确类型的包
    CHECK(input_port->receivePacket(packet1) == true);
    CHECK(input_port->size() == 1);
    CHECK(input_port->hasCapacity() == true);

    // 测试接收到容量上限
    CHECK(input_port->receivePacket(packet2) == true);
    CHECK(input_port->size() == 2);
    CHECK(input_port->hasCapacity() == false);

    // 测试超过容量
    CHECK(input_port->receivePacket(packet3) == false);
    CHECK(input_port->size() == 2);

    // 测试接收错误类型的包
    CHECK(input_port->receivePacket(info_packet) == false);

    // 测试获取包
    auto peek = input_port->peekPacket();
    CHECK(peek == packet1);
    CHECK(input_port->size() == 2);

    auto pop = input_port->popPacket();
    CHECK(pop == packet1);
    CHECK(input_port->size() == 1);
    CHECK(input_port->hasCapacity() == true);
}

TEST_CASE("Output Port") {
    using namespace sim;

    auto output_port = std::make_shared<OutputPort>("output", VoidPacket::type_id);
    auto input_port1 = std::make_shared<InputPort>("input1", VoidPacket::type_id, 2);
    auto input_port2 = std::make_shared<InputPort>("input2", VoidPacket::type_id, 1);
    auto packet = std::make_shared<VoidPacket>(1);
    auto info_packet = std::make_shared<InfoPacket>(2, PacketID(), "test");

    // 测试连接
    output_port->connectTo(input_port1);
    output_port->connectTo(input_port2);
    CHECK(output_port->getConnectedPorts().size() == 2);

    // 测试发送正确类型的包
    CHECK(output_port->sendPacket(packet) == true);
    CHECK(input_port1->size() == 1);
    CHECK(input_port2->size() == 1);

    // 测试发送错误类型的包
    CHECK(output_port->sendPacket(info_packet) == false);

    // 测试发送到已满的端口
    auto packet2 = std::make_shared<VoidPacket>(3);
    CHECK(output_port->sendPacket(packet2) == true);
    CHECK(input_port1->size() == 2);
    CHECK(input_port2->size() == 1);  // input_port2已满，不会接收

    // 测试检查目标端口是否已满
    CHECK(output_port->areAllInputPortsFull() == true);  // 所有输入端口都已满

    // 断开连接
    output_port->disconnectFrom(input_port1);
    CHECK(output_port->getConnectedPorts().size() == 1);
}

TEST_CASE("Port Serialization") {
    using namespace sim;

    // 创建输入端口并添加一些包
    auto input_port = std::make_shared<InputPort>("input", VoidPacket::type_id, 2);
    auto packet1 = std::make_shared<VoidPacket>(1);
    auto packet2 = std::make_shared<VoidPacket>(2);
    input_port->receivePacket(packet1);
    input_port->receivePacket(packet2);

    // 序列化
    auto json = nlohmann::json::parse(input_port->serialize());
    CHECK(json["name"] == "input");
    CHECK(json["accepted_type_id"] == VoidPacket::type_id);
    CHECK(json["capacity"] == 2);
    CHECK(json["packets"].size() == 2);

    // 创建新的端口并反序列化
    auto new_port = std::make_shared<InputPort>("temp", 0, 0);
    new_port->deserialize(json.dump());
    CHECK(new_port->getName() == "input");
    CHECK(new_port->getAcceptedTypeID() == VoidPacket::type_id);
    CHECK(new_port->getCapacity() == 2);
}

// 生产者节点
class ProducerNode : public sim::Node {
public:
    static constexpr sim::TypeID type_id = sim::generateTypeID("ProducerNode");

    ProducerNode(sim::NodeID id) : sim::Node(id, sim::VoidPacket::type_id) {
        // 添加一个输出端口
        addOutputPort("out", sim::VoidPacket::type_id);
    }

    sim::TypeID getTypeID() const override { return type_id; }

protected:
    void onTick() override {
        // 在tick阶段不做任何事
    }

    void onTock() override {
        // 在tock阶段生成一个新的VoidPacket并发送
        auto packet = spawnPacket<sim::VoidPacket>();
        if (packet) {
            auto out_port = getOutputPort("out");
            if (out_port) {
                out_port->sendPacket(packet);
            }
        }
    }

private:
    std::shared_ptr<sim::Node> createNodeFromJson(const nlohmann::json& j) override { return nullptr; }
    std::shared_ptr<sim::Packet> createPacketFromJson(const nlohmann::json& j) override { return nullptr; }
};

// 消费者节点
class ConsumerNode : public sim::Node {
public:
    static constexpr sim::TypeID type_id = sim::generateTypeID("ConsumerNode");

    ConsumerNode(sim::NodeID id) : sim::Node(id, sim::VoidPacket::type_id) {
        // 添加一个输入端口
        addInputPort("in", sim::VoidPacket::type_id, 5);  // 设置缓冲区大小为5
    }

    sim::TypeID getTypeID() const override { return type_id; }

    // 获取已消费的数据包数量
    size_t getConsumedCount() const { return consumed_count_; }

protected:
    void onTick() override {
        // 在tick阶段不做任何事
    }

    void onTock() override {
        // 在tock阶段消费一个数据包
        auto in_port = getInputPort("in");
        if (in_port && in_port->size() > 0) {
            auto packet = in_port->popPacket();
            if (packet) {
                consumed_count_++;
            }
        }
    }

private:
    std::shared_ptr<sim::Node> createNodeFromJson(const nlohmann::json& j) override { return nullptr; }
    std::shared_ptr<sim::Packet> createPacketFromJson(const nlohmann::json& j) override { return nullptr; }
    size_t consumed_count_ = 0;
};

TEST_CASE("Connected Nodes Communication") {
    using namespace sim;

    // 创建生产者和消费者节点
    auto producer = std::make_shared<ProducerNode>(1);
    auto consumer = std::make_shared<ConsumerNode>(2);

    // 获取并连接端口
    auto producer_out = producer->getOutputPort("out");
    auto consumer_in = consumer->getInputPort("in");
    REQUIRE(producer_out != nullptr);
    REQUIRE(consumer_in != nullptr);
    producer_out->connectTo(consumer_in);

    // 运行几个周期的仿真
    for (int i = 0; i < 3; i++) {
        producer->tick();
        consumer->tick();
        producer->tock();
        consumer->tock();
    }

    // 验证结果
    CHECK(consumer->getConsumedCount() == 3);  // 应该消费了3个数据包
    CHECK(consumer_in->size() == 0);  // 输入端口应该为空
} 