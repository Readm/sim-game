#include "server.h"
#include "network_factory.h"
#include <iostream>
#include <thread>
#include <chrono>

// 简化的doctest实现
#define TEST_CASE(name) void test_case_##name()
#define TEST_CASE_FIXTURE(name) void test_case_##name()
#define MESSAGE(text) std::cout << "[INFO] " << text << std::endl
#define CHECK(condition) if(!(condition)) { std::cout << "[FAIL] Check failed: " << #condition << std::endl; } else { std::cout << "[PASS] Check passed: " << #condition << std::endl; }
#define CHECK_FALSE(condition) if((condition)) { std::cout << "[FAIL] Check failed: " << #condition << " should be false" << std::endl; } else { std::cout << "[PASS] Check passed: " << #condition << " is false" << std::endl; }
#define CHECK_EQ(a, b) if((a) != (b)) { std::cout << "[FAIL] Check failed: " << #a << " != " << #b << std::endl; } else { std::cout << "[PASS] Check passed: " << #a << " == " << #b << std::endl; }

// 测试用例1：创建生产者-消费者网络
TEST_CASE_FIXTURE(producer_consumer_network_test) {
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

// 测试用例2：生产者-消费者网络模拟
TEST_CASE_FIXTURE(producer_consumer_simulation_test) {
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

// 测试用例3：自动模拟测试
TEST_CASE_FIXTURE(auto_simulation_test) {
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

// 测试用例4：从JSON加载和重置测试
TEST_CASE_FIXTURE(load_json_and_reset_test) {
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
    bool loaded = server.loadNetworkFromJson(stateJson);
    CHECK(loaded);
    
    // 验证状态已恢复
    json loadedState = server.getNetworkState();
    std::cout << "加载后的状态: " << loadedState.dump() << std::endl;
    
    server.stop();
}

// 运行所有测试
int main(int argc, char** argv) {
    std::cout << "======== 网络模拟测试 ========" << std::endl;
    
    std::cout << "\n--- 创建生产者-消费者网络测试 ---" << std::endl;
    test_case_producer_consumer_network_test();
    
    std::cout << "\n--- 生产者-消费者网络模拟测试 ---" << std::endl;
    test_case_producer_consumer_simulation_test();
    
    std::cout << "\n--- 自动模拟测试 ---" << std::endl;
    test_case_auto_simulation_test();
    
    std::cout << "\n--- 从JSON加载和重置测试 ---" << std::endl;
    test_case_load_json_and_reset_test();
    
    std::cout << "\n======== 测试完成 ========" << std::endl;
    
    return 0;
} 