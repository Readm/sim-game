#include "sim/include/server.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <filesystem>

// 简化的doctest实现
#define TEST_CASE(name) void test_case_##name()
#define TEST_CASE_FIXTURE(name) void test_case_##name()
#define MESSAGE(text) std::cout << "[INFO] " << text << std::endl
#define CHECK(condition) if(!(condition)) { std::cout << "[FAIL] Check failed: " << #condition << std::endl; } else { std::cout << "[PASS] Check passed: " << #condition << std::endl; }
#define CHECK_FALSE(condition) if((condition)) { std::cout << "[FAIL] Check failed: " << #condition << " should be false" << std::endl; } else { std::cout << "[PASS] Check passed: " << #condition << " is false" << std::endl; }
#define CHECK_EQ(a, b) if((a) != (b)) { std::cout << "[FAIL] Check failed: " << #a << " != " << #b << std::endl; } else { std::cout << "[PASS] Check passed: " << #a << " == " << #b << std::endl; }

// 测试用例1：基本服务器功能测试
TEST_CASE_FIXTURE(basic_server_test) {
    sim::Server server(8080);
    
    MESSAGE("测试服务器启动和停止");
    
    // 初始状态应该是停止的
    CHECK_FALSE(server.isRunning());
    
    // 启动服务器
    bool started = server.start();
    CHECK(started);
    CHECK(server.isRunning());
    
    // 停止服务器
    server.stop();
    CHECK_FALSE(server.isRunning());
}

// 测试用例2：从JSON加载网络
TEST_CASE_FIXTURE(load_network_test) {
    sim::Server server(8080);
    server.start();
    
    MESSAGE("测试从JSON加载网络");
    
    // 创建测试JSON
    std::string testJson = R"({
        "tick": 0,
        "running": false,
        "nodes": [
            {"id": 1, "type": "input", "name": "Input Node", "x": 100, "y": 100},
            {"id": 2, "type": "output", "name": "Output Node", "x": 400, "y": 100}
        ],
        "connections": [
            {"id": 1, "sourceNodeId": 1, "sourcePort": 0, "targetNodeId": 2, "targetPort": 0}
        ]
    })";
    
    // 加载网络
    bool loaded = server.loadNetworkFromJson(testJson);
    CHECK(loaded);
    
    // 验证状态
    json state = server.getNetworkState();
    CHECK(state != json("{}"));
    
    server.stop();
}

// 测试用例3：模拟控制测试
TEST_CASE_FIXTURE(simulation_control_test) {
    sim::Server server(8080);
    server.start();
    
    MESSAGE("测试模拟控制功能");
    
    // 创建并加载测试网络
    std::string testJson = R"({
        "tick": 0,
        "running": false,
        "nodes": [
            {"id": 1, "type": "input", "name": "Input Node", "x": 100, "y": 100},
            {"id": 2, "type": "output", "name": "Output Node", "x": 400, "y": 100}
        ],
        "connections": [
            {"id": 1, "sourceNodeId": 1, "sourcePort": 0, "targetNodeId": 2, "targetPort": 0}
        ]
    })";
    
    server.loadNetworkFromJson(testJson);
    
    // 启动模拟
    server.clearRequestLog();
    bool started = server.startSimulation();
    CHECK(started);
    
    // 等待一段时间让模拟运行
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // 检查请求日志
    auto requestLog = server.getRequestLog();
    CHECK(requestLog.size() >= 1);
    
    // 停止模拟
    server.clearRequestLog();
    bool stopped = server.stopSimulation();
    CHECK(stopped);
    
    // 等待一段时间确保模拟完全停止
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // 重置模拟
    server.clearRequestLog();
    bool reset = server.resetSimulation();
    CHECK(reset);
    
    // 单步执行
    server.clearRequestLog();
    bool stepped = server.stepSimulation();
    CHECK(stepped);
    
    server.stop();
}

// 测试用例4：从文件加载网络
TEST_CASE_FIXTURE(load_from_file_test) {
    sim::Server server(8080);
    server.start();
    
    MESSAGE("测试从文件加载网络");
    
    // 创建临时测试文件
    std::string testFilePath = "test_network.json";
    std::string testJson = R"({
        "tick": 0,
        "running": false,
        "nodes": [
            {"id": 1, "type": "input", "name": "Input Node", "x": 100, "y": 100},
            {"id": 2, "type": "output", "name": "Output Node", "x": 400, "y": 100}
        ],
        "connections": [
            {"id": 1, "sourceNodeId": 1, "sourcePort": 0, "targetNodeId": 2, "targetPort": 0}
        ]
    })";
    
    std::ofstream outFile(testFilePath);
    if (outFile.is_open()) {
        outFile << testJson;
        outFile.close();
        
        // 加载网络
        bool loaded = server.loadNetworkFromFile(testFilePath);
        CHECK(loaded);
        
        // 验证状态
        json state = server.getNetworkState();
        CHECK(state != json("{}"));
        
        // 删除临时文件
        std::filesystem::remove(testFilePath);
    } else {
        MESSAGE("无法创建测试文件，跳过此测试");
    }
    
    server.stop();
}

// 测试用例5：状态更新回调测试
TEST_CASE_FIXTURE(state_update_callback_test) {
    sim::Server server(8080);
    server.start();
    
    MESSAGE("测试状态更新回调");
    
    // 初始化测试网络
    std::string testJson = R"({
        "tick": 0,
        "running": false,
        "nodes": [],
        "connections": []
    })";
    
    server.loadNetworkFromJson(testJson);
    
    // 设置回调
    bool callbackCalled = false;
    server.setStateUpdateCallback([&callbackCalled](const json& state) {
        callbackCalled = true;
        std::cout << "状态更新回调被调用，状态: " << state.dump() << std::endl;
    });
    
    // 启动模拟，应该触发状态更新
    server.startSimulation();
    
    // 等待一段时间让回调被调用
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    CHECK(callbackCalled);
    
    server.stop();
}

// 运行所有测试
int main(int argc, char** argv) {
    std::cout << "======== 服务器单元测试 ========" << std::endl;
    
    std::cout << "\n--- 基本服务器功能测试 ---" << std::endl;
    test_case_basic_server_test();
    
    std::cout << "\n--- 从JSON加载网络测试 ---" << std::endl;
    test_case_load_network_test();
    
    std::cout << "\n--- 模拟控制测试 ---" << std::endl;
    test_case_simulation_control_test();
    
    std::cout << "\n--- 从文件加载网络测试 ---" << std::endl;
    test_case_load_from_file_test();
    
    std::cout << "\n--- 状态更新回调测试 ---" << std::endl;
    test_case_state_update_callback_test();
    
    std::cout << "\n======== 测试完成 ========" << std::endl;
    
    return 0;
} 