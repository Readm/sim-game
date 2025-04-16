#include "../network_viewer.h"
#include "mock_server.h"
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

// 全局测试对象
MockServer g_server(8080);
NetworkViewer g_viewer;

void setup_test() {
    // 启动模拟服务器
    g_server.start();
    
    // 设置初始网络状态
    json initialState = json("{\"tick\":0,\"running\":false,\"nodes\":[],\"connections\":[]}");
    g_server.setNetworkState(initialState);
    
    // 等待服务器完全启动
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 连接到服务器
    g_viewer.connectToServer();
}

void teardown_test() {
    // 断开与服务器的连接
    g_viewer.disconnectFromServer();
    
    // 停止服务器
    g_server.stop();
    
    // 清理请求日志
    g_server.clearRequestLog();
}

// 辅助函数：模拟运行模拟并生成状态更新
void runSimulationSteps(int steps) {
    for (int i = 0; i < steps; i++) {
        // 更新tick和running状态
        json state = json("{\"tick\":" + std::to_string(i+1) + 
                          ",\"running\":true,\"nodes\":[],\"connections\":[]}");
        
        // 更新服务器状态
        g_server.setNetworkState(state);
        
        // 直接更新NetworkViewer，模拟收到WebSocket消息
        g_viewer.updateNetworkState(state.dump());
        
        // 等待短暂时间
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// 测试用例1：实时状态更新测试
TEST_CASE_FIXTURE(realtime_update_test) {
    setup_test();
    MESSAGE("测试WebSocket实时状态更新");
    
    // 初始状态
    CHECK_FALSE(g_viewer.isSimulationRunning());
    CHECK_EQ(g_viewer.getCurrentTick(), 0);
    
    // 启动模拟
    g_viewer.startSimulation();
    
    // 检查状态已更新
    CHECK(g_viewer.isSimulationRunning());
    
    // 运行几步模拟，每步都会通过WebSocket更新状态
    MESSAGE("运行5步模拟...");
    runSimulationSteps(5);
    
    // 检查tick是否已更新
    MESSAGE("验证tick更新...");
    CHECK_EQ(g_viewer.getCurrentTick(), 5);
    
    // 停止模拟
    g_viewer.stopSimulation();
    CHECK_FALSE(g_viewer.isSimulationRunning());
    
    teardown_test();
}

// 运行所有测试
int main(int argc, char** argv) {
    std::cout << "======== WebSocket测试 ========" << std::endl;
    
    // 初始化NetworkViewer
    g_viewer.init();
    
    std::cout << "\n--- 实时状态更新测试 ---" << std::endl;
    test_case_realtime_update_test();
    
    // 清理NetworkViewer
    g_viewer.shutdown();
    
    std::cout << "\n======== 测试完成 ========" << std::endl;
    
    return 0;
} 