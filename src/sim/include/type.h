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
using PacketID = uint64_t;
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