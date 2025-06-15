#pragma once

#include "node.h"
#include "packet.h"
#include <queue>
#include <mutex>
#include <vector>

namespace sim {

class FIFONode : public Node {
public:
    static constexpr TypeID type_id = generateTypeID("FIFONode");
    inline static bool type_registered = TypeRegistry::getInstance().registerType(type_id, "FIFONode");

    explicit FIFONode(NodeID id = 0, size_t capacity = 10) 
        : Node(id, VoidPacket::type_id), capacity_(capacity) {}

    TypeID getTypeID() const override { return type_id; }
    
    // 获取当前队列大小
    size_t getSize() const { 
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size(); 
    }
    
    // 获取队列容量
    size_t getCapacity() const { return capacity_; }
    
    // 设置队列容量
    void setCapacity(size_t capacity) { capacity_ = capacity; }
    
    // 检查队列是否为空
    bool isEmpty() const { 
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty(); 
    }
    
    // 检查队列是否已满
    bool isFull() const { 
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size() >= capacity_; 
    }

protected:
    void onTick() override {
        // 从输入端口接收数据包并尝试入队
        auto in_port = getInputPort("in");
        if (in_port && in_port->isValid()) {
            auto packet = in_port->popPacket();
            if (packet) {
                std::lock_guard<std::mutex> lock(mutex_);
                if (queue_.size() < capacity_) {
                    queue_.push(packet);
                }
            }
        }
    }

    void onTock() override {
        // 尝试从队列中取出数据包并发送到输出端口
        auto out_port = getOutputPort("out");
        if (out_port && !queue_.empty()) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!queue_.empty()) {
                auto packet = queue_.front();
                queue_.pop();
                out_port->sendPacket(packet);
            }
        }
    }

private:
    std::queue<std::shared_ptr<Packet>> queue_;
    mutable std::mutex mutex_;
    size_t capacity_;

    // --- 新增序列化实现 ---
    void serializeImpl(nlohmann::json& j) const override {
        j["capacity"] = capacity_;
        // 序列化队列内容
        std::vector<nlohmann::json> queue_json;
        std::lock_guard<std::mutex> lock(mutex_);
        std::queue<std::shared_ptr<Packet>> tmp = queue_;
        while (!tmp.empty()) {
            auto pkt = tmp.front();
            tmp.pop();
            if (pkt) queue_json.push_back(nlohmann::json::parse(pkt->serialize()));
        }
        j["queue"] = queue_json;
    }

    void deserializeImpl(const nlohmann::json& j) override {
        if (j.contains("capacity")) capacity_ = j["capacity"];
        // 反序列化队列内容
        if (j.contains("queue")) {
            std::lock_guard<std::mutex> lock(mutex_);
            std::queue<std::shared_ptr<Packet>> empty;
            std::swap(queue_, empty); // 清空
            for (const auto& pkt_json : j["queue"]) {
                auto pkt = std::make_shared<VoidPacket>();
                pkt->deserialize(pkt_json.dump());
                queue_.push(pkt);
            }
        }
    }

    std::shared_ptr<Node> createNodeFromJson(const nlohmann::json& json) override {
        auto node = std::make_shared<FIFONode>();
        node->deserialize(json.dump());
        return node;
    }

    std::shared_ptr<Packet> createPacketFromJson(const nlohmann::json& json) override {
        auto packet = std::make_shared<VoidPacket>();
        packet->deserialize(json.dump());
        return packet;
    }
};

} // namespace sim 