#include "pch.h"
#include "packet.h"

// 模拟获取有效载荷指针的函数
void* GetPayloadPointer() {
    static int payload_data = 12345;  // 示例有效载荷
    return &payload_data;
}

// 测试: Packet 类的构造函数
TEST(PacketTest, ConstructorTest) {
    size_t type_id = 1;
    std::string header_data = "header";
    void* payload_data = GetPayloadPointer();

    Packet Packet(type_id, header_data, payload_data);

    EXPECT_EQ(Packet.GetTypeID(), type_id);
    EXPECT_EQ(Packet.GetHeaderData(), header_data);
    EXPECT_EQ(Packet.GetPayloadData(), payload_data);
}

// 测试: 注册和查询TypeID对应的HeaderStructure
TEST(PacketTest, HeaderStructureRegistryTest) {
    size_t type_id = 1;

    // 创建HeaderStructure
    HeaderStructure header_structure;
    header_structure.fields.push_back({ "Field1", 4 });
    header_structure.fields.push_back({ "Field2", 8 });

    // 注册这个TypeID的Header结构
    Packet::HeaderStructureRegistry::GetInstance().RegisterType(type_id, header_structure);

    // 获取并验证Header结构
    const HeaderStructure& retrieved_structure = Packet::GetHeaderStructure(type_id);

    ASSERT_EQ(retrieved_structure.fields.size(), 2);
    EXPECT_EQ(retrieved_structure.fields[0].name, "Field1");
    EXPECT_EQ(retrieved_structure.fields[0].size, 4);
    EXPECT_EQ(retrieved_structure.fields[1].name, "Field2");
    EXPECT_EQ(retrieved_structure.fields[1].size, 8);
}

// 测试: 查询不存在的TypeID
TEST(PacketTest, InvalidTypeIDTest) {
    EXPECT_THROW(Packet::GetHeaderStructure(999), std::runtime_error);  // 999是未注册的TypeID
}

