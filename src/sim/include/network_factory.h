#pragma once

#include "network.h"
#include "producer_consumer.h"
#include <memory>
#include <string>

namespace sim {

/**
 * @brief 网络工厂类，用于创建和配置仿真网络
 */
class NetworkFactory {
public:
    /**
     * @brief 创建一个简单的生产者-消费者网络
     * 
     * 创建一个包含一个生产者和一个消费者的网络，并连接它们
     * 
     * @param producer_name 生产者名称
     * @param consumer_name 消费者名称
     * @return 创建的网络对象
     */
    static std::shared_ptr<Network> createProducerConsumerNetwork(
        const std::string& producer_name = "生产者",
        const std::string& consumer_name = "消费者") {
        
        // 创建网络根节点
        auto network = std::make_shared<Network>();
        
        // 创建生产者节点 (ID = 1)
        auto producer = std::make_shared<ProducerNode>(1, producer_name);
        
        // 创建消费者节点 (ID = 2)
        auto consumer = std::make_shared<ConsumerNode>(2, consumer_name);
        
        // 添加节点到网络
        network->addChild(producer);
        network->addChild(consumer);
        
        // 连接生产者和消费者
        auto producer_out = producer->getOutputPort("out");
        auto consumer_in = consumer->getInputPort("in");
        if (producer_out && consumer_in) {
            producer_out->connectTo(consumer_in);
        }
        
        return network;
    }
    
    /**
     * @brief 将网络序列化为JSON字符串
     * 
     * @param network 要序列化的网络
     * @return JSON字符串
     */
    static std::string serializeNetwork(const std::shared_ptr<Network>& network) {
        if (network) {
            return network->serialize();
        }
        return "{}";
    }
    
    /**
     * @brief 从JSON字符串反序列化网络
     * 
     * @param json_str JSON字符串
     * @return 反序列化的网络对象
     */
    static std::shared_ptr<Network> deserializeNetwork(const std::string& json_str) {
        auto network = std::make_shared<Network>();
        network->deserialize(json_str);
        return network;
    }
};

} // namespace sim 