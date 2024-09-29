#ifndef NODE_H
#define NODE_H


#include <vector>
#include <mutex>
#include "packet.h"  // 引入Packet类


/* TODO：目前使用比较简单的实现，后面可以做性能优化，例如ECS等方式。
 * TODO：增加关于 GUI 中的信息：不同TypeID对应不同颜色等。
 * TODO：修改要求：每个Field必须可以动态地定义（游戏要求），因此不可以使用编译时定义的（模板）。
 */

// Node类
class Node
{
public:
	// 构造函数，接受TypeID、NodeID、code指针、输入端口TypeID数组、输出端口TypeID数组
	Node(size_t type_id, size_t node_id, void* code_ptr,
	     const std::vector<size_t>& input_ports, const std::vector<size_t>& output_ports)
		: type_id_(type_id), node_id_(node_id), code_(code_ptr),
		  input_ports_(input_ports), output_ports_(output_ports)
	{
	}

	// 获取TypeID
	size_t GetTypeID() const { return type_id_; }

	// 获取NodeID
	size_t GetNodeID() const { return node_id_; }

	// 获取code指针
	void* GetCodePtr() const { return code_; }

	// 获取输入端口的TypeID列表
	const std::vector<size_t>& GetInputPorts() const { return input_ports_; }

	// 获取输出端口的TypeID列表
	const std::vector<size_t>& GetOutputPorts() const { return output_ports_; }

	// 获取Packet数组
	const std::vector<Packet>& GetPackets() const { return packets_; }

	// 向Node的Packet数组中添加Packet
	void AddPacket(const Packet& packet)
	{
		packets_.push_back(packet);
	}

	// 清空Packet数组
	void ClearPackets()
	{
		packets_.clear();
	}

private:
	size_t type_id_; // TypeID，表示这个Node的类型
	size_t node_id_; // 唯一标识符NodeID
	void* code_; // 指向代码的指针
	std::vector<size_t> input_ports_; // 输入端口TypeID列表
	std::vector<size_t> output_ports_; // 输出端口TypeID列表
	std::vector<Packet> packets_; // 包含的Packet数组
};

#endif // NODE_H
