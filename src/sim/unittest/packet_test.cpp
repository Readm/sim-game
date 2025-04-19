#include "doctest/doctest.h"
#include "sim/include/packet.h"
#include "sim/include/node.h"

// 测试用的Node类
class TestNode : public sim::Node {
public:
    static constexpr sim::TypeID type_id = sim::generateTypeID("TestNode");
    inline static bool type_registered = sim::TypeRegistry::getInstance().registerType(type_id, "TestNode");

    TestNode(sim::NodeID id = 0, sim::TypeID packet_type_id = sim::VoidPacket::type_id) 
        : Node(id, packet_type_id) {}
    sim::TypeID getTypeID() const override { return type_id; }

private:
    std::shared_ptr<sim::Node> createNodeFromJson(const nlohmann::json& j) override {
        auto node = std::make_shared<TestNode>();
        node->deserialize(j.dump());
        return node;
    }
    std::shared_ptr<sim::Packet> createPacketFromJson(const nlohmann::json& j) override {
        auto packet = std::make_shared<sim::VoidPacket>();
        packet->deserialize(j.dump());
        return packet;
    }
};

// 测试用的Packet类型
struct TestPacket : public sim::Packet {
    static constexpr sim::TypeID type_id = sim::generateTypeID("TestPacket");
    inline static bool type_registered = sim::TypeRegistry::getInstance().registerType(type_id, "TestPacket");

    TestPacket(sim::NodeID src_node_id = 0, sim::PacketID id = sim::PacketID()) 
        : Packet(src_node_id, id) {}
    sim::TypeID getTypeID() const override { return type_id; }
};

TEST_CASE("Packet Base Class") {
    using namespace sim;

    TestNode node(123, TestPacket::type_id);
    auto packet = node.spawnPacket<TestPacket>();
    REQUIRE(packet != nullptr);
    CHECK(packet->getSrcNodeID() == 123);
    CHECK(packet->getPacketID().local_seq > 0);
    CHECK(packet->getTypeID() == TestPacket::type_id);
}

TEST_CASE("VoidPacket") {
    using namespace sim;

    TestNode node(456, VoidPacket::type_id);
    auto packet = node.spawnPacket<VoidPacket>();
    REQUIRE(packet != nullptr);
    CHECK(packet->getSrcNodeID() == 456);
    CHECK(packet->getPacketID().local_seq > 0);
    CHECK(packet->getTypeID() == VoidPacket::type_id);
}

TEST_CASE("InfoPacket") {
    using namespace sim;

    TestNode node(789, InfoPacket::type_id);
    auto packet = node.spawnPacket<InfoPacket>();
    REQUIRE(packet != nullptr);
    CHECK(packet->getSrcNodeID() == 789);
    CHECK(packet->getPacketID().local_seq > 0);
    CHECK(packet->getTypeID() == InfoPacket::type_id);
    CHECK(static_cast<InfoPacket*>(packet.get())->getInfo() == "");

    // 测试修改info
    static_cast<InfoPacket*>(packet.get())->setInfo("New Info");
    CHECK(static_cast<InfoPacket*>(packet.get())->getInfo() == "New Info");
}

TEST_CASE("Packet Serialization") {
    using namespace sim;

    // 测试VoidPacket序列化
    TestNode node(123, VoidPacket::type_id);
    auto void_packet = node.spawnPacket<VoidPacket>();
    auto void_json = nlohmann::json::parse(void_packet->serialize());
    CHECK(void_json["type_id"] == VoidPacket::type_id);
    CHECK(void_json["src_node_id"] == 123);
    CHECK(void_json["packet_id"]["node_id"] == void_packet->getPacketID().node_id);
    CHECK(void_json["packet_id"]["local_seq"] == void_packet->getPacketID().local_seq);

    // 测试InfoPacket序列化
    TestNode info_node(456, InfoPacket::type_id);
    auto info_packet = info_node.spawnPacket<InfoPacket>();
    static_cast<InfoPacket*>(info_packet.get())->setInfo("Test Info");
    printf("info_packet: %s\n", info_packet->serialize().c_str());
    auto info_json = nlohmann::json::parse(info_packet->serialize());
    CHECK(info_json["type_id"] == InfoPacket::type_id);
    CHECK(info_json["src_node_id"] == 456);
    CHECK(info_json["packet_id"]["node_id"] == info_packet->getPacketID().node_id);
    CHECK(info_json["packet_id"]["local_seq"] == info_packet->getPacketID().local_seq);
    CHECK(info_json["info"] == "Test Info");

    // 测试反序列化
    auto new_packet = std::make_shared<InfoPacket>();
    new_packet->deserialize(info_json.dump());
    CHECK(new_packet->getSrcNodeID() == 456);
    CHECK(static_cast<InfoPacket*>(new_packet.get())->getInfo() == "Test Info");
} 