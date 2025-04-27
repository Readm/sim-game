#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "server.h"
#include "network_factory.h"
#include <iostream>
#include <thread>
#include <chrono>

TEST_CASE("测试创建生产者-消费者网络") {
    // 创建服务器
    sim::Server server(8080);
    server.start();
    
    MESSAGE("测试创建生产者-消费者网络");
    
    // 通过API创建网络
    server.processApiRequest("POST", "/api/network/create/producer-consumer", "");
    
    // 验证网络状态
    json state = server.getNetworkState();
    CHECK(state != json("{}"));
    
    // 显示初始状态
    std::cout << "初始网络状态: " << state.dump() << std::endl;
    
    server.stop();
}

TEST_CASE("测试生产者-消费者网络模拟") {
    // 创建服务器
    sim::Server server(8080);
    server.start();
    
    MESSAGE("测试生产者-消费者网络模拟");
    
    // 创建网络
    server.processApiRequest("POST", "/api/network/create/producer-consumer", "");
    
    // 运行几个周期的模拟
    for (int i = 0; i < 5; i++) {
        std::cout << "\n执行第 " << (i+1) << " 次模拟周期..." << std::endl;
        bool stepped = server.stepSimulation();
        CHECK(stepped);
        
        // 获取并显示状态
        json state = server.getNetworkState();
        std::cout << "网络状态: " << state.dump() << std::endl;
        
        // 稍微等待一下，让日志输出完成
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    server.stop();
}

TEST_CASE("测试自动模拟") {
    // 创建服务器
    sim::Server server(8080);
    server.start();
    
    MESSAGE("测试自动模拟");
    
    // 创建网络
    server.processApiRequest("POST", "/api/network/create/producer-consumer", "");
    
    // 创建状态更新回调
    bool callbackCalled = false;
    std::string lastState;
    server.setStateUpdateCallback([&callbackCalled, &lastState](const json& state) {
        callbackCalled = true;
        lastState = state.dump();
        std::cout << "状态更新回调被调用: " << lastState << std::endl;
    });
    
    // 启动自动模拟
    bool started = server.startSimulation();
    CHECK(started);
    
    // 运行一段时间
    std::cout << "自动模拟运行中，等待3秒..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // 验证回调是否被调用
    CHECK(callbackCalled);
    
    // 停止模拟
    bool stopped = server.stopSimulation();
    CHECK(stopped);
    
    server.stop();
}

TEST_CASE("测试从JSON加载和重置") {
    // 创建服务器
    sim::Server server(8080);
    server.start();
    
    MESSAGE("测试从JSON加载和重置");
    
    // 创建网络
    server.processApiRequest("POST", "/api/network/create/producer-consumer", "");
    
    // 运行一个周期
    server.stepSimulation();
    
    // 获取当前状态
    json state = server.getNetworkState();
    std::string stateJson = state.dump();
    std::cout << "运行一个周期后的状态: " << stateJson << std::endl;
    
    // 重置模拟
    bool reset = server.resetSimulation();
    CHECK(reset);
    
    // 验证状态已重置
    json resetState = server.getNetworkState();
    std::cout << "重置后的状态: " << resetState.dump() << std::endl;
    
    // 加载之前保存的状态
    try {
        json stateToLoad = json::parse(stateJson);
        bool loaded = server.loadNetworkFromJson(stateToLoad);
        CHECK(loaded);
        
        // 验证状态已恢复
        json loadedState = server.getNetworkState();
        std::cout << "加载后的状态: " << loadedState.dump() << std::endl;
    } catch (const json::exception& e) {
        FAIL("JSON解析失败: ", e.what());
    }
    
    server.stop();
} 