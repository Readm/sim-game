#include "pch.h"
#include "node.h"
#include "packet.h"

// ģ��code��ָ�뺯��
void* GetMockCodePointer() {
    static int mock_code = 42;  // ģ��Ĵ�������
    return &mock_code;
}

// ����: Node ��Ĺ��캯��
TEST(NodeTest, ConstructorTest) {
    size_t type_id = 1;
    size_t node_id = 101;
    void* code_ptr = GetMockCodePointer();
    std::vector<size_t> input_ports = { 2, 3 };  // ����˿�����ID
    std::vector<size_t> output_ports = { 4 };    // ����˿�����ID

    Node node(type_id, node_id, code_ptr, input_ports, output_ports);

    EXPECT_EQ(node.GetTypeID(), type_id);
    EXPECT_EQ(node.GetNodeID(), node_id);
    EXPECT_EQ(node.GetCodePtr(), code_ptr);
    EXPECT_EQ(node.GetInputPorts().size(), 2);
    EXPECT_EQ(node.GetOutputPorts().size(), 1);
}

// ����: Node �� Packet ����
TEST(NodeTest, PacketManagementTest) {
    size_t type_id = 1;
    size_t node_id = 101;
    void* code_ptr = GetMockCodePointer();
    std::vector<size_t> input_ports = { 2, 3 };  // ����˿�����ID
    std::vector<size_t> output_ports = { 4 };    // ����˿�����ID

    Node node(type_id, node_id, code_ptr, input_ports, output_ports);

    // ����һ��ģ��Packet
    size_t packet_type_id = 2;
    std::string header_data = "test_header";
    void* payload_data = GetMockCodePointer();  // ģ����Ч�غ�����
    Packet packet(packet_type_id, header_data, payload_data);

    // ���Packet��Node��
    node.AddPacket(packet);

    // ��֤Node�е�Packet
    ASSERT_EQ(node.GetPackets().size(), 1);
    EXPECT_EQ(node.GetPackets()[0].GetTypeID(), packet_type_id);
    EXPECT_EQ(node.GetPackets()[0].GetHeaderData(), header_data);

    // ���Node�е�Packet
    node.ClearPackets();
    EXPECT_EQ(node.GetPackets().size(), 0);
}

// ����: Node �����������˿�TypeID
TEST(NodeTest, PortTypeIDTest) {
    size_t type_id = 1;
    size_t node_id = 101;
    void* code_ptr = GetMockCodePointer();
    std::vector<size_t> input_ports = { 2, 3 };  // ����˿�����ID
    std::vector<size_t> output_ports = { 4 };    // ����˿�����ID

    Node node(type_id, node_id, code_ptr, input_ports, output_ports);

    // ��֤����˿�TypeID
    const std::vector<size_t>& input_port_ids = node.GetInputPorts();
    ASSERT_EQ(input_port_ids.size(), 2);
    EXPECT_EQ(input_port_ids[0], 2);
    EXPECT_EQ(input_port_ids[1], 3);

    // ��֤����˿�TypeID
    const std::vector<size_t>& output_port_ids = node.GetOutputPorts();
    ASSERT_EQ(output_port_ids.size(), 1);
    EXPECT_EQ(output_port_ids[0], 4);
}
