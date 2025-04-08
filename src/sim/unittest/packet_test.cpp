#include "doctest/doctest.h"
#include "../include/packet.h"

// 测试用的Packet类型
class TestPacket : public sim::Packet {
public:
    static constexpr sim::TypeID type_id = sim::generateTypeID("TestPacket");
    inline static bool type_registered = sim::TypeRegistry::getInstance().registerType(type_id, "TestPacket");

    using Packet::Packet;  // 继承基类的构造函数
    sim::TypeID getTypeID() const override { return type_id; }
};

TEST_CASE("Packet Base Class") {
    using namespace sim;

    TestPacket packet(123);
    CHECK(packet.getSrcNodeID() == 123);
    CHECK(packet.getPacketID() > 0);
    CHECK(packet.getTypeID() == TestPacket::type_id);
}

TEST_CASE("VoidPacket") {
    using namespace sim;

    VoidPacket packet(456);
    CHECK(packet.getSrcNodeID() == 456);
    CHECK(packet.getPacketID() > 0);
    CHECK(packet.getTypeID() == VoidPacket::type_id);
}

TEST_CASE("InfoPacket") {
    using namespace sim;

    InfoPacket packet(789, "Test Info");
    CHECK(packet.getSrcNodeID() == 789);
    CHECK(packet.getPacketID() > 0);
    CHECK(packet.getTypeID() == InfoPacket::type_id);
    CHECK(packet.getInfo() == "Test Info");

    // 测试修改info
    packet.setInfo("New Info");
    CHECK(packet.getInfo() == "New Info");
}

TEST_CASE("Packet Serialization") {
    using namespace sim;

    // 测试VoidPacket序列化
    VoidPacket void_packet(123);
    auto void_json = void_packet.toJson();
    CHECK(void_json["type_id"] == VoidPacket::type_id);
    CHECK(void_json["src_node_id"] == 123);
    CHECK(void_json["packet_id"] == void_packet.getPacketID());

    // 测试InfoPacket序列化
    InfoPacket info_packet(456, "Test Info");
    auto info_json = info_packet.toJson();
    CHECK(info_json["type_id"] == InfoPacket::type_id);
    CHECK(info_json["src_node_id"] == 456);
    CHECK(info_json["packet_id"] == info_packet.getPacketID());
    CHECK(info_json["info"] == "Test Info");

    // 测试反序列化
    InfoPacket new_packet;
    new_packet.fromJson(info_json);
    CHECK(new_packet.getSrcNodeID() == 456);
    CHECK(new_packet.getInfo() == "Test Info");
} 