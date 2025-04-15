/**
 * @file type.h
 * @brief 定义了仿真系统中的基础类型系统
 * 
 * 该文件定义了以下内容：
 * - 基础ID类型（NodeID、PacketID、PayloadID、TypeID）
 * - 序列化方法枚举
 * - 并行化方法枚举
 * - 模拟模式枚举
 * - 类型注册系统
 * 
 * 特性：
 * - 提供类型安全的ID生成和管理
 * - 支持运行时类型注册和查询
 * - 实现了类型名称到ID的映射
 * - 提供了类型注册的宏支持
 */

#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include <vector>

namespace sim {

// 基础ID类型
using NodeID = uint64_t;
// PacketID现在是一个结构体，包含NodeID和本地序列号
struct PacketID {
    NodeID node_id;      // 生成该Packet的Node的ID
    uint64_t local_seq;  // 本地序列号

    // 构造函数
    PacketID() : node_id(0), local_seq(0) {}
    PacketID(NodeID nid, uint64_t seq) : node_id(nid), local_seq(seq) {}

    // 相等运算符
    bool operator==(const PacketID& other) const {
        return node_id == other.node_id && local_seq == other.local_seq;
    }

    // 转换为字符串（用于调试）
    std::string toString() const {
        return std::to_string(node_id) + ":" + std::to_string(local_seq);
    }
};

using PayloadID = uint64_t;
using TypeID = uint64_t;

// 序列化方法枚举
enum class SerializationMethod {
    JSON,
    BINARY,
    PROTOBUF
};

// 并行化方法枚举
enum class ParallelizationMethod {
    NONE,
    OPENMP,
    THREAD_POOL
};

// 模拟模式枚举
enum class SimulationMode {
    FASTEST,    // 最快模式：不断simulate直到结束
    STEP,       // 单步模式：每次执行一个Tick或Tock，等待输入
    TRACE       // 跟踪模式：每个Tock后输出序列化结果
};

// 类型ID生成函数
constexpr TypeID generateTypeID(const char* name) {
    // 简单的字符串哈希函数
    TypeID hash = 5381;
    while (*name) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(*name);
        name++;
    }
    return hash;
}

// 类型注册系统
class TypeRegistry {
public:
    static TypeRegistry& getInstance() {
        static TypeRegistry instance;
        return instance;
    }

    bool registerType(TypeID id, const std::string& name) {
        auto it = typeMap_.find(id);
        if (it != typeMap_.end()) {
            if (it->second != name) {
                return false; // 类型ID冲突
            }
            return true; // 已经注册过相同的类型
        }
        typeMap_[id] = name;
        return true;
    }

    const std::string& getTypeName(TypeID id) const {
        static const std::string unknown = "UNKNOWN";
        auto it = typeMap_.find(id);
        return it != typeMap_.end() ? it->second : unknown;
    }

private:
    TypeRegistry() = default;
    std::unordered_map<TypeID, std::string> typeMap_;
};

// 类型注册宏
#define REGISTER_TYPE(type_name) \
    static constexpr TypeID type_id = sim::generateTypeID(#type_name); \
    inline static bool type_registered = sim::TypeRegistry::getInstance().registerType(type_id, #type_name)

} // namespace sim 

// 为PacketID添加哈希函数支持
namespace std {
    template<>
    struct hash<sim::PacketID> {
        size_t operator()(const sim::PacketID& id) const {
            // 组合两个字段的哈希值
            return hash<sim::NodeID>()(id.node_id) ^ 
                   (hash<uint64_t>()(id.local_seq) << 1);
        }
    };
} 