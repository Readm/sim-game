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
}

void teardown_test() {
    // 断开与服务器的连接
    g_viewer.disconnectFromServer();
    
    // 停止服务器
    g_server.stop();
    
    // 清理请求日志
    g_server.clearRequestLog();
}

// 测试用例1：连接测试
TEST_CASE_FIXTURE(connection_test) {
    setup_test();
    MESSAGE("测试前端连接到后端服务器");
    
    // 初始状态应该是未连接
    CHECK_FALSE(g_viewer.isConnected());
    
    // 连接服务器
    bool connected = g_viewer.connectToServer("http://localhost:8080");
    CHECK(connected);
    CHECK(g_viewer.isConnected());
    
    // 检查连接后服务器是否收到了请求
    auto requestLog = g_server.getRequestLog();
    CHECK(requestLog.size() >= 1);
    
    teardown_test();
}

// 测试用例2：模拟控制测试
TEST_CASE_FIXTURE(simulation_control_test) {
    setup_test();
    MESSAGE("测试通过前端控制后端模拟器");
    
    // 连接服务器
    CHECK(g_viewer.connectToServer());
    
    // 启动模拟
    g_server.clearRequestLog();
    g_viewer.startSimulation();
    
    // 检查服务器是否收到了启动请求
    auto requestLog = g_server.getRequestLog();
    CHECK(requestLog.size() >= 1);
    
    // 检查服务器状态是否已更新
    json state = g_server.getNetworkState();
    CHECK(state == true); // 检查running字段
    
    // 停止模拟
    g_server.clearRequestLog();
    g_viewer.stopSimulation();
    
    // 检查服务器是否收到了停止请求
    requestLog = g_server.getRequestLog();
    CHECK(requestLog.size() >= 1);
    
    // 检查服务器状态是否已更新
    state = g_server.getNetworkState();
    CHECK_FALSE(state == true); // 检查running字段
    
    teardown_test();
}

// 测试用例3：错误处理测试
TEST_CASE_FIXTURE(error_handling_test) {
    setup_test();
    MESSAGE("测试前端处理后端错误");
    
    // 设置服务器错误率
    g_server.setErrorRate(1.0); // 100%错误率
    
    // 尝试连接
    bool connected = g_viewer.connectToServer();
    CHECK_FALSE(connected);
    CHECK_FALSE(g_viewer.isConnected());
    
    // 重置错误率
    g_server.setErrorRate(0.0);
    
    // 再次尝试连接
    connected = g_viewer.connectToServer();
    CHECK(connected);
    CHECK(g_viewer.isConnected());
    
    // 设置服务器延迟
    g_server.setResponseDelay(500); // 500ms延迟
    
    // 尝试操作，应该仍然成功但会有延迟
    auto start = std::chrono::steady_clock::now();
    g_viewer.startSimulation();
    auto end = std::chrono::steady_clock::now();
    
    // 检查操作是否花费了至少设定的延迟时间
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    CHECK(duration >= 500);
    
    teardown_test();
}

// 运行所有测试
int main(int argc, char** argv) {
    std::cout << "======== 网络集成测试 ========" << std::endl;
    
    // 初始化NetworkViewer
    g_viewer.init();
    
    std::cout << "\n--- 连接测试 ---" << std::endl;
    test_case_connection_test();
    
    std::cout << "\n--- 模拟控制测试 ---" << std::endl;
    test_case_simulation_control_test();
    
    std::cout << "\n--- 错误处理测试 ---" << std::endl;
    test_case_error_handling_test();
    
    // 清理NetworkViewer
    g_viewer.shutdown();
    
    std::cout << "\n======== 测试完成 ========" << std::endl;
    
    return 0;
} 