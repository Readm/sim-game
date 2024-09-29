#pragma once
#ifndef PACKET_H
#define PACKET_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
/*
 * Packet 是游戏中可嵌套的最小的数据结构，他由Header和Payload组成。
 * Header 是当前模块所处理的域段。
 * Payload 是当前模块不解析，更低一层的 Packet。
 */

/* TODO：目前使用比较简单的实现，后面可以做性能优化，例如ECS等方式。
 * TODO：增加关于 GUI 中的信息：不同TypeID对应不同颜色等。
 * TODO：修改要求：每个Field必须可以动态地定义（游戏要求），因此不可以使用编译时定义的（模板）。
 */

// 定义一个结构表示Header中的域段信息
struct HeaderField {
    std::string name;  // 域段名称
    size_t size;       // 域段大小（以字节为单位）
};

// Header结构：包含多个域段
struct HeaderStructure {
    std::vector<HeaderField> fields;  // 这个Header中包含的域段
};

// Packet类
class Packet {
public:
    // 构造函数
    Packet(size_t type_id, const std::string& header_data, void* payload_data)
        : type_id_(type_id), header_data_(header_data), payload_data_(payload_data) {}

    // 获取TypeID
    size_t GetTypeID() const { return type_id_; }

    // 获取HeaderData
    const std::string& GetHeaderData() const { return header_data_; }

    // 获取PayloadData指针
    void* GetPayloadData() const { return payload_data_; }

    // 静态方法，用于查询TypeID对应的Header结构
    static const HeaderStructure& GetHeaderStructure(size_t type_id) {
        return HeaderStructureRegistry::GetInstance().GetHeaderStructure(type_id);
    }

private:
    size_t type_id_;           // TypeID，表示这个包的类型
    std::string header_data_;  // HeaderData，包含打包的Header信息
    void* payload_data_;       // PayloadData，指向实际的Payload数据
public:
    // 单例类，用于管理TypeID到HeaderStructure的映射
    class HeaderStructureRegistry {
    public:
        // 获取单例实例
        static HeaderStructureRegistry& GetInstance() {
            static HeaderStructureRegistry instance;
            return instance;
        }

        // 禁止拷贝和赋值
        HeaderStructureRegistry(const HeaderStructureRegistry&) = delete;
        HeaderStructureRegistry& operator=(const HeaderStructureRegistry&) = delete;

        // 注册新的TypeID对应的Header结构
        void RegisterType(size_t type_id, const HeaderStructure& header_structure) {
            std::lock_guard<std::mutex> lock(mutex_);
            type_to_header_structure_[type_id] = header_structure;
        }

        // 获取TypeID对应的Header结构
        const HeaderStructure& GetHeaderStructure(size_t type_id) const {
            auto it = type_to_header_structure_.find(type_id);
            if (it != type_to_header_structure_.end()) {
                return it->second;
            }
            else {
                throw std::runtime_error("TypeID not found in HeaderStructureRegistry");
            }
        }

    
        // 构造函数（单例模式）
        HeaderStructureRegistry() {}
	private:
        // 维护TypeID到HeaderStructure的映射
        std::unordered_map<size_t, HeaderStructure> type_to_header_structure_;

        // 线程安全的互斥锁
        mutable std::mutex mutex_;
    };
};

#endif // PACKET_H
