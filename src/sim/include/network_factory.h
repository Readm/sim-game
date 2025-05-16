#pragma once

#include "network.h"
#include "producer_consumer.h"
#include <memory>
#include <string>

namespace sim {

/**
 * @brief Network factory class, used to create and configure simulation networks
 */
class NetworkFactory {
public:
    /**
     * @brief Create a simple producer-consumer network
     * 
     * Create a network containing one producer and one consumer, and connect them
     * 
     * @param producer_name Producer name
     * @param consumer_name Consumer name
     * @return Created network object
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
     * @brief Serialize network to JSON string
     * 
     * @param network Network to serialize
     * @return JSON string
     */
    static std::string serializeNetwork(const std::shared_ptr<Network>& network) {
        if (network) {
            return network->serialize();
        }
        return "{}";
    }
    
    /**
     * @brief Deserialize network from JSON string
     * 
     * @param json_str JSON string
     * @return Deserialized network object
     */
    static std::shared_ptr<Network> deserializeNetwork(const std::string& json_str) {
        auto network = std::make_shared<Network>();
        network->deserialize(json_str);
        return network;
    }
};

} // namespace sim 