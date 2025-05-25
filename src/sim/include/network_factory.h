/**
 * @file network_factory.h
 * @brief 定义了仿真系统中的网络工厂类
 * 
 * 该文件实现了网络工厂模式，提供以下功能：
 * - 创建预定义的网络拓扑结构
 * - 生产者-消费者网络的快速创建
 * - 网络的序列化和反序列化
 * - 网络配置的标准化管理
 * 
 * NetworkFactory类简化了常见网络拓扑的创建过程，
 * 提供了一套标准的API来构建和管理仿真网络。
 */

#pragma once

#include "network.h"
#include "producer_consumer.h"
#include <memory>
#include <string>

namespace sim {

/**
 * @brief 网络工厂类，用于创建和配置仿真网络
 * 
 * NetworkFactory提供了一套静态方法来创建常见的网络拓扑结构。
 * 该类实现了工厂模式，简化了网络创建过程，并确保网络配置的一致性。
 * 
 * 主要功能：
 * - 创建预定义的网络拓扑（如生产者-消费者网络）
 * - 自动配置节点连接
 * - 提供网络序列化/反序列化支持
 * - 支持网络配置的标准化管理
 */
class NetworkFactory {
public:
    /**
     * @brief 创建简单的生产者-消费者网络
     * 
     * 创建一个包含一个生产者和一个消费者的网络，并将它们连接起来
     * 
     * @param producer_name 生产者名称
     * @param consumer_name 消费者名称
     * @return 创建的网络对象
     */
    static std::shared_ptr<Network> createProducerConsumerNetwork(
        const std::string& producer_name = "Producer",
        const std::string& consumer_name = "Consumer") {
        
        // Create network root node
        auto network = std::make_shared<Network>();
        
        // Create producer node (ID = 1)
        auto producer = std::make_shared<ProducerNode>(1, producer_name);
        
        // Create consumer node (ID = 2)
        auto consumer = std::make_shared<ConsumerNode>(2, consumer_name);
        
        // Add nodes to network
        network->addChild(producer);
        network->addChild(consumer);
        
        // Connect producer and consumer
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