#include "doctest/doctest.h"
#include "common.h"
#include "fifo_node.h"
#include "packet.h"
#include "producer_consumer.h"
#include <chrono>
#include <thread>

namespace sim {

TEST_CASE("FIFO Node Basic Operations") {
    auto fifo = std::make_shared<FIFONode>(1, 3);  // 创建容量为3的FIFO节点
    
    // 添加输入输出端口
    auto in_port = fifo->addInputPort("in", VoidPacket::type_id, 2);
    auto out_port = fifo->addOutputPort("out", VoidPacket::type_id);
    
    CHECK(fifo->getCapacity() == 3);
    CHECK(fifo->isEmpty());
    CHECK(!fifo->isFull());
    CHECK(fifo->getSize() == 0);
}

TEST_CASE("FIFO Node Enqueue Operations") {
    auto fifo = std::make_shared<FIFONode>(1, 3);
    auto in_port = fifo->addInputPort("in", VoidPacket::type_id, 2);
    auto out_port = fifo->addOutputPort("out", VoidPacket::type_id);
    
    // 测试入队操作
    for (int i = 0; i < 3; ++i) {
        auto packet = std::make_shared<VoidPacket>(i);
        in_port->receivePacket(packet);
        fifo->tick();  // 触发入队
        CHECK(fifo->getSize() == i + 1);
    }
    
    // 测试队列已满的情况
    auto overflow_packet = std::make_shared<VoidPacket>(4);
    in_port->receivePacket(overflow_packet);
    fifo->tick();
    CHECK(fifo->getSize() == 3);  // 大小应该保持不变
    CHECK(fifo->isFull());
}

TEST_CASE("FIFO Node Dequeue Operations") {
    auto fifo = std::make_shared<FIFONode>(1, 3);
    auto in_port = fifo->addInputPort("in", VoidPacket::type_id, 2);
    auto out_port = fifo->addOutputPort("out", VoidPacket::type_id);
    
    // 先填充队列
    for (int i = 0; i < 3; ++i) {
        auto packet = std::make_shared<VoidPacket>(i);
        in_port->receivePacket(packet);
        fifo->tick();
    }
    
    // 测试出队操作
    for (int i = 0; i < 3; ++i) {
        fifo->tock();  // 触发出队
        CHECK(fifo->getSize() == 2 - i);
    }
    
    // 测试空队列出队
    fifo->tock();
    CHECK(fifo->getSize() == 0);
    CHECK(fifo->isEmpty());
}

TEST_CASE("FIFO Node Serialization") {
    auto fifo = std::make_shared<FIFONode>(1, 5);
    auto in_port = fifo->addInputPort("in", VoidPacket::type_id, 2);
    auto out_port = fifo->addOutputPort("out", VoidPacket::type_id);
    
    // 添加一些数据
    for (int i = 0; i < 3; ++i) {
        auto packet = std::make_shared<VoidPacket>(i);
        in_port->receivePacket(packet);
        fifo->tick();
    }
    
    // 序列化
    auto json = nlohmann::json::parse(fifo->serialize());
    CHECK(json["type_id"] == FIFONode::type_id);
    CHECK(json["node_id"] == 1);
    
    // 反序列化
    auto new_fifo = std::make_shared<FIFONode>();
    new_fifo->deserialize(json.dump());
    
    CHECK(new_fifo->getNodeID() == 1);
    CHECK(new_fifo->getCapacity() == 5);
    CHECK(new_fifo->getSize() == 3);  // 队列中的数据应该被保留
}

TEST_CASE("FIFO Node Complex Network") {
    // 创建多个FIFO节点
    auto fifo1 = std::make_shared<FIFONode>(1, 5);
    auto fifo2 = std::make_shared<FIFONode>(2, 5);
    auto fifo3 = std::make_shared<FIFONode>(3, 5);
    
    // 创建生产者节点
    auto producer1 = std::make_shared<ProducerNode>(4);
    auto producer2 = std::make_shared<ProducerNode>(5);
    auto producer3 = std::make_shared<ProducerNode>(6);
    
    // 创建消费者节点
    auto consumer1 = std::make_shared<ConsumerNode>(7);
    auto consumer2 = std::make_shared<ConsumerNode>(8);
    
    // 设置端口
    auto p1_out = producer1->addOutputPort("out", VoidPacket::type_id);
    auto p2_out = producer2->addOutputPort("out", VoidPacket::type_id);
    auto p3_out = producer3->addOutputPort("out", VoidPacket::type_id);
    
    auto f1_in = fifo1->addInputPort("in", VoidPacket::type_id, 2);
    auto f1_out = fifo1->addOutputPort("out", VoidPacket::type_id);
    
    auto f2_in = fifo2->addInputPort("in", VoidPacket::type_id, 2);
    auto f2_out = fifo2->addOutputPort("out", VoidPacket::type_id);
    
    auto f3_in = fifo3->addInputPort("in", VoidPacket::type_id, 2);
    auto f3_out = fifo3->addOutputPort("out", VoidPacket::type_id);
    
    auto c1_in = consumer1->addInputPort("in", VoidPacket::type_id);
    auto c2_in = consumer2->addInputPort("in", VoidPacket::type_id);
    
    // 连接节点
    p1_out->connectTo(f1_in);
    p2_out->connectTo(f2_in);
    p3_out->connectTo(f3_in);
    
    f1_out->connectTo(c1_in);
    f2_out->connectTo(c1_in);
    f3_out->connectTo(c2_in);
    
    // 运行多个时钟周期
    for (int i = 0; i < 10; ++i) {
        // 生产者生成数据
        producer1->tick();
        producer2->tick();
        producer3->tick();
        
        // FIFO节点处理入队
        fifo1->tick();
        fifo2->tick();
        fifo3->tick();
        
        // FIFO节点处理出队
        fifo1->tock();
        fifo2->tock();
        fifo3->tock();
        
        // 消费者处理数据
        consumer1->tick();
        consumer2->tick();
        
        // 验证FIFO节点的状态
        CHECK(fifo1->getSize() <= fifo1->getCapacity());
        CHECK(fifo2->getSize() <= fifo2->getCapacity());
        CHECK(fifo3->getSize() <= fifo3->getCapacity());
    }
    
    // 验证最终状态
    CHECK(consumer1->getConsumedCount() > 0);
    CHECK(consumer2->getConsumedCount() > 0);
}

} // namespace sim 