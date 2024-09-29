#include "pch.h"
#include "node.h"
#include "packet.h"

// 模拟code的指针函数
void* GetMockCodePointer() {
    static int mock_code = 42;  // 模拟的代码数据
    return &mock_code;
}

// 测试: Node 类的构造函数
TEST(NodeTest, ConstructorTest) {
    size_t type_id = 1;
    size_t node_id = 101;
    void* code_ptr = GetMockCodePointer();
    std::vector<size_t> input_ports = { 2, 3 };  // 输入端口类型ID
    std::vector<size_t> output_ports = { 4 };    // 输出端口类型ID

    Node node(type_id, node_id, code_ptr, input_ports, output_ports);

    EXPECT_EQ(node.GetTypeID(), type_id);
    EXPECT_EQ(node.GetNodeID(), node_id);
    EXPECT_EQ(node.GetCodePtr(), code_ptr);
    EXPECT_EQ(node.GetInputPorts().size(), 2);
    EXPECT_EQ(node.GetOutputPorts().size(), 1);
}

// 测试: Node 的 Packet 操作
TEST(NodeTest, PacketManagementTest) {
    size_t type_id = 1;
    size_t node_id = 101;
    void* code_ptr = GetMockCodePointer();
    std::vector<size_t> input_ports = { 2, 3 };  // 输入端口类型ID
    std::vector<size_t> output_ports = { 4 };    // 输出端口类型ID

    Node node(type_id, node_id, code_ptr, input_ports, output_ports);

    // 创建一个模拟Packet
    size_t packet_type_id = 2;
    std::string header_data = "test_header";
    void* payload_data = GetMockCodePointer();  // 模拟有效载荷数据
    Packet packet(packet_type_id, header_data, payload_data);

    // 添加Packet到Node中
    node.AddPacket(packet);

    // 验证Node中的Packet
    ASSERT_EQ(node.GetPackets().size(), 1);
    EXPECT_EQ(node.GetPackets()[0].GetTypeID(), packet_type_id);
    EXPECT_EQ(node.GetPackets()[0].GetHeaderData(), header_data);

    // 清空Node中的Packet
    node.ClearPackets();
    EXPECT_EQ(node.GetPackets().size(), 0);
}

// 测试: Node 的输入和输出端口TypeID
TEST(NodeTest, PortTypeIDTest) {
    size_t type_id = 1;
    size_t node_id = 101;
    void* code_ptr = GetMockCodePointer();
    std::vector<size_t> input_ports = { 2, 3 };  // 输入端口类型ID
    std::vector<size_t> output_ports = { 4 };    // 输出端口类型ID

    Node node(type_id, node_id, code_ptr, input_ports, output_ports);

    // 验证输入端口TypeID
    const std::vector<size_t>& input_port_ids = node.GetInputPorts();
    ASSERT_EQ(input_port_ids.size(), 2);
    EXPECT_EQ(input_port_ids[0], 2);
    EXPECT_EQ(input_port_ids[1], 3);

    // 验证输出端口TypeID
    const std::vector<size_t>& output_port_ids = node.GetOutputPorts();
    ASSERT_EQ(output_port_ids.size(), 1);
    EXPECT_EQ(output_port_ids[0], 4);
}
