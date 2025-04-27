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

    // 测试获取包（需要先tick和tock）
    input_port->tick();
    CHECK(input_port->isValid() == true);
    auto peek = input_port->peekPacket();
    CHECK(peek == packet1);
    CHECK(input_port->size() == 2);

    auto pop = input_port->popPacket();
    CHECK(pop == packet1);
    CHECK(input_port->size() == 1);
    CHECK(input_port->hasCapacity() == true);

    input_port->tock();
}

TEST_CASE("Output Port") {
    using namespace sim;

    auto output_port = std::make_shared<OutputPort>("output", VoidPacket::type_id);
    auto input_port1 = std::make_shared<InputPort>("input1", VoidPacket::type_id, 2);
    auto input_port2 = std::make_shared<InputPort>("input2", VoidPacket::type_id, 1);
    auto packet = std::make_shared<VoidPacket>(1);
    auto info_packet = std::make_shared<InfoPacket>(2, PacketID(), "test");

    // 连接端口
    output_port->connectTo(input_port1);
    output_port->connectTo(input_port2);
    CHECK(output_port->getConnectedPorts().size() == 2);

    // 测试发送正确类型的包（需要先tick）
    output_port->tick();
    input_port1->tick();
    input_port2->tick();
    CHECK(output_port->sendPacket(packet) == true);
    CHECK(input_port1->size() == 1);
    CHECK(input_port2->size() == 1);

    output_port->tock();
    input_port1->tock();
    input_port2->tock();

    // 测试发送错误类型的包
    output_port->tick();
    CHECK(output_port->sendPacket(info_packet) == false);
    output_port->tock();

    // 测试发送到已满的端口
    auto packet2 = std::make_shared<VoidPacket>(3);
    output_port->tick();
    input_port1->tick();
    input_port2->tick();
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
        // 在tick阶段检查输出端口是否ready
        auto out_port = getOutputPort("out");
        if (out_port && out_port->isReady()) {
            should_generate_ = true;  // 只设置标志，不生成数据包
        } else {
            should_generate_ = false;
        }
    }

    void onTock() override {
        // 在tock阶段生成并发送数据包
        if (should_generate_) {
            auto out_port = getOutputPort("out");
            if (out_port) {
                auto packet = spawnPacket<sim::VoidPacket>();
                out_port->sendPacket(packet);
            }
        }
    }

private:
    std::shared_ptr<sim::Node> createNodeFromJson(const nlohmann::json& j) override { return nullptr; }
    std::shared_ptr<sim::Packet> createPacketFromJson(const nlohmann::json& j) override { return nullptr; }
    bool should_generate_ = false;  // 标记是否应该生成新的数据包
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
        // 在tick阶段检查输入端口是否有数据
        auto in_port = getInputPort("in");
        if (in_port && in_port->isValid()) {
            can_consume_ = true;
        } else {
            can_consume_ = false;
        }
    }

    void onTock() override {
        // 在tock阶段消费数据包
        if (can_consume_) {
            auto in_port = getInputPort("in");
            if (in_port) {
                auto packet = in_port->popPacket();
                if (packet) {
                    consumed_count_++;
                }
            }
        }
    }

private:
    std::shared_ptr<sim::Node> createNodeFromJson(const nlohmann::json& j) override { return nullptr; }
    std::shared_ptr<sim::Packet> createPacketFromJson(const nlohmann::json& j) override { return nullptr; }
    size_t consumed_count_ = 0;
    bool can_consume_ = false;
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
    for (int i = 0; i < 6; i++) {  // 增加仿真周期数，因为现在需要更多周期来完成数据传输
        producer->tick();
        consumer->tick();
        producer->tock();
        consumer->tock();
    }

    // 验证结果
    CHECK(consumer->getConsumedCount() == 5);  // 应该消费了5个数据包
    CHECK(consumer_in->size() == 1);  // 输入端口应该还有一个数据包
}

TEST_CASE("Port Handshake Mechanism") {
    using namespace sim;

    auto output_port = std::make_shared<OutputPort>("output", VoidPacket::type_id, 2);
    auto input_port = std::make_shared<InputPort>("input", VoidPacket::type_id, 2);
    auto packet = std::make_shared<VoidPacket>(1);

    // 连接端口
    output_port->connectTo(input_port);

    // 初始状态检查
    CHECK(output_port->isReady() == true);  // 输出端口有容量
    CHECK(input_port->isValid() == false);  // 输入端口没有数据

    // 第一个周期：尝试发送数据
    output_port->tick();
    input_port->tick();
    CHECK(output_port->sendPacket(packet) == true);  // 可以发送，因为output port已经tick了

    output_port->tock();
    input_port->tock();

    // 第二个周期：添加数据到输入端口
    input_port->receivePacket(packet);
    
    output_port->tick();
    input_port->tick();
    CHECK(input_port->isValid() == true);  // 现在输入端口有数据了
    CHECK(output_port->isReady() == true); // 输出端口仍然有容量

    output_port->tock();
    input_port->tock();

    // 第三个周期：尝试从输入端口获取数据
    output_port->tick();
    input_port->tick();
    auto received_packet = input_port->popPacket();
    CHECK(received_packet == packet);  // 成功获取数据

    output_port->tock();
    input_port->tock();

    // 检查TickTock计数
    CHECK(input_port->getTickTock() == 3);
    CHECK(output_port->getTickTock() == 3);
}

TEST_CASE("Port Handshake with Full Buffer") {
    using namespace sim;

    auto output_port = std::make_shared<OutputPort>("output", VoidPacket::type_id, 1);
    auto input_port = std::make_shared<InputPort>("input", VoidPacket::type_id, 1);
    auto packet1 = std::make_shared<VoidPacket>(1);
    auto packet2 = std::make_shared<VoidPacket>(2);

    // 连接端口
    output_port->connectTo(input_port);

    // 填满输入端口
    input_port->receivePacket(packet1);

    // 第一个周期：检查状态
    output_port->tick();
    input_port->tick();
    CHECK(input_port->isValid() == true);   // 输入端口有数据
    CHECK(input_port->hasCapacity() == false); // 输入端口已满
    CHECK(output_port->isReady() == true);  // 输出端口有容量

    output_port->tock();
    input_port->tock();

    // 第二个周期：尝试发送数据到已满的输入端口
    output_port->tick();
    input_port->tick();
    CHECK(output_port->sendPacket(packet2) == false); // 不能发送，因为输入端口已满

    output_port->tock();
    input_port->tock();

    // 第三个周期：清空输入端口并再次尝试发送
    output_port->tick();
    input_port->tick();
    auto received_packet = input_port->popPacket();
    CHECK(received_packet == packet1);

    output_port->tock();
    input_port->tock();

    // 第四个周期：现在应该可以发送了
    output_port->tick();
    input_port->tick();
    CHECK(output_port->sendPacket(packet2) == true); // 现在可以发送了

    output_port->tock();
    input_port->tock();
} 