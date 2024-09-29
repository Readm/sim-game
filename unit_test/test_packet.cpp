#include "pch.h"
#include "packet.h"

// ģ���ȡ��Ч�غ�ָ��ĺ���
void* GetPayloadPointer() {
    static int payload_data = 12345;  // ʾ����Ч�غ�
    return &payload_data;
}

// ����: Packet ��Ĺ��캯��
TEST(PacketTest, ConstructorTest) {
    size_t type_id = 1;
    std::string header_data = "header";
    void* payload_data = GetPayloadPointer();

    Packet Packet(type_id, header_data, payload_data);

    EXPECT_EQ(Packet.GetTypeID(), type_id);
    EXPECT_EQ(Packet.GetHeaderData(), header_data);
    EXPECT_EQ(Packet.GetPayloadData(), payload_data);
}

// ����: ע��Ͳ�ѯTypeID��Ӧ��HeaderStructure
TEST(PacketTest, HeaderStructureRegistryTest) {
    size_t type_id = 1;

    // ����HeaderStructure
    HeaderStructure header_structure;
    header_structure.fields.push_back({ "Field1", 4 });
    header_structure.fields.push_back({ "Field2", 8 });

    // ע�����TypeID��Header�ṹ
    Packet::HeaderStructureRegistry::GetInstance().RegisterType(type_id, header_structure);

    // ��ȡ����֤Header�ṹ
    const HeaderStructure& retrieved_structure = Packet::GetHeaderStructure(type_id);

    ASSERT_EQ(retrieved_structure.fields.size(), 2);
    EXPECT_EQ(retrieved_structure.fields[0].name, "Field1");
    EXPECT_EQ(retrieved_structure.fields[0].size, 4);
    EXPECT_EQ(retrieved_structure.fields[1].name, "Field2");
    EXPECT_EQ(retrieved_structure.fields[1].size, 8);
}

// ����: ��ѯ�����ڵ�TypeID
TEST(PacketTest, InvalidTypeIDTest) {
    EXPECT_THROW(Packet::GetHeaderStructure(999), std::runtime_error);  // 999��δע���TypeID
}

