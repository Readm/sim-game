#ifndef SIM_TYPE_H
#define SIM_TYPE_H

#include <cstdint>

using ElementID_t = uint64_t;
using TypeID_t = uint64_t;

// Node ID类型定义
using NodeTypeId = TypeID_t;
using NodeId = ElementID_t;

// Packet ID类型定义
using PacketTypeId = TypeID_t;
struct PacketId {
    ElementID_t src_node_id;
    ElementID_t packet_id;
};


#endif // SIM_TYPE_H 