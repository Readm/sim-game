#include "server.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <httplib.h>
#include <memory>
#include <future>

using json = nlohmann::json;

// 使用标准的doctest框架
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

// 异步启动服务器的辅助函数
std::future<bool> startServerAsync(sim::Server& server) {
    return std::async(std::launch::async, [&server]() {
        std::cout << "开始异步启动服务器..." << std::endl;
        bool result = server.start();
        std::cout << "服务器异步启动" << (result ? "成功" : "失败") << std::endl;
        return result;
    });
}

// 异步关闭服务器的辅助函数
std::future<void> stopServerAsync(sim::Server& server) {
    return std::async(std::launch::async, [&server]() {
        std::cout << "开始异步关闭服务器..." << std::endl;
        server.stop();
        std::cout << "服务器异步关闭完成" << std::endl;
    });
}

// 测试用例1：基本服务器功能测试
TEST_CASE("basic_server_api_test") {
    std::cout << "\n=== 开始基本服务器功能测试 ===" << std::endl;
    
    // 创建服务器实例
    sim::Server server(8080);
    std::cout << "服务器实例已创建" << std::endl;
    
    SUBCASE("测试服务器启动和健康检查") {
        std::cout << "\n--- 测试服务器启动和健康检查 ---" << std::endl;
        
        // 异步启动服务器
        auto start_future = startServerAsync(server);
        
        // 等待服务器启动
        std::cout << "等待服务器启动..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 创建HTTP客户端
        httplib::Client cli("http://localhost:8080");
        std::cout << "HTTP客户端已创建" << std::endl;
        
        // 测试健康检查API
        std::cout << "发送健康检查请求..." << std::endl;
        auto health_res = cli.Get("/api/health");
        CHECK(health_res != nullptr);
        CHECK(health_res->status == 200);
        
        // 解析响应
        auto health_json = json::parse(health_res->body);
        CHECK(health_json["status"] == "ok");
        std::cout << "健康检查通过" << std::endl;
        
        // 等待服务器启动完成
        std::cout << "等待服务器启动完成..." << std::endl;
        start_future.wait();
    }
    
    SUBCASE("测试服务器状态API") {
        std::cout << "\n--- 测试服务器状态API ---" << std::endl;
        
        // 确保服务器正在运行
        if (!server.isRunning()) {
            std::cout << "服务器未运行，重新启动..." << std::endl;
            auto start_future = startServerAsync(server);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            start_future.wait();
        }
        
        // 创建HTTP客户端
        httplib::Client cli("http://localhost:8080");
        
        // 先创建一个简单的网络
        auto create_res = cli.Post("/api/network/create/producer-consumer");
        CHECK(create_res != nullptr);
        CHECK(create_res->status == 200);
        
        // 等待网络创建完成
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 测试获取网络状态API
        auto state_res = cli.Get("/api/network/state");
        CHECK(state_res != nullptr);
        CHECK(state_res->status == 200);
        
        // 打印原始响应内容
        std::cout << "原始响应内容:\n" << state_res->body << std::endl;
        
        // 解析响应
        json state_json;
        std::string clean_body;
        try {
            // 处理响应内容
            clean_body = state_res->body;
            
            // 移除开头和结尾的引号
            if (clean_body.front() == '"' && clean_body.back() == '"') {
                clean_body = clean_body.substr(1, clean_body.length() - 2);
            }
            
            // 处理转义字符
            std::string::size_type pos = 0;
            while ((pos = clean_body.find("\\\"", pos)) != std::string::npos) {
                clean_body.replace(pos, 2, "\"");
                pos += 1;
            }
            
            // 找到第一个完整的 JSON 对象
            size_t first_brace = clean_body.find('{');
            size_t last_brace = clean_body.rfind('}');
            
            if (first_brace != std::string::npos && last_brace != std::string::npos && first_brace < last_brace) {
                clean_body = clean_body.substr(first_brace, last_brace - first_brace + 1);
            }
            
            std::cout << "清理后的内容:\n" << clean_body << std::endl;
            
            // 解析 JSON
            state_json = json::parse(clean_body);
            std::cout << "解析后的JSON类型: " << state_json.type_name() << std::endl;
            std::cout << "解析后的JSON内容:\n" << state_json.dump(4) << std::endl;
            
            // 检查基本字段
            CHECK(state_json.is_object());
            CHECK(state_json["running"].is_boolean());
            CHECK(state_json["tick_tock"].is_number());
            CHECK(state_json["node_id"].is_number());
            CHECK(state_json["type_id"].is_number());
            CHECK(state_json["packet_type_id"].is_number());
            CHECK(state_json["next_packet_seq"].is_number());
            
            // 检查节点数组
            CHECK(state_json["children"].is_array());
            CHECK(state_json["children"].size() == 2); // 生产者和消费者两个节点
            
            // 检查生产者节点
            const auto& producer = state_json["children"][0];
            CHECK(producer["name"] == "生产者");
            CHECK(producer["output_ports"].is_object());
            CHECK(producer["output_ports"]["out"].is_object());
            CHECK(producer["produced_count"].is_number());
            
            // 检查消费者节点
            const auto& consumer = state_json["children"][1];
            CHECK(consumer["name"] == "消费者");
            CHECK(consumer["input_ports"].is_object());
            CHECK(consumer["input_ports"]["in"].is_object());
            CHECK(consumer["consumed_count"].is_number());
        } catch (const json::parse_error& e) {
            std::cerr << "JSON解析错误: " << e.what() << std::endl;
            std::cerr << "错误位置: " << e.byte << std::endl;
            std::cerr << "清理后的内容: " << clean_body << std::endl;
            FAIL("JSON解析失败");
        } catch (const json::type_error& e) {
            std::cerr << "JSON类型错误: " << e.what() << std::endl;
            FAIL("JSON类型错误");
        }
    }
    
    SUBCASE("测试服务器关闭API") {
        std::cout << "\n--- 测试服务器关闭API ---" << std::endl;
        
        // 确保服务器正在运行
        if (!server.isRunning()) {
            std::cout << "服务器未运行，重新启动..." << std::endl;
            auto start_future = startServerAsync(server);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            start_future.wait();
        }
        
        // 创建HTTP客户端
        httplib::Client cli("http://localhost:8080");
        std::cout << "HTTP客户端已创建" << std::endl;
        
        // 测试关闭服务器API
        std::cout << "发送关闭服务器请求..." << std::endl;
        auto shutdown_res = cli.Post("/api/server/shutdown");
        std::cout << "关闭服务器请求收到..." << std::endl;
        CHECK(shutdown_res != nullptr);
        CHECK(shutdown_res->status == 200);
        
        // 解析响应
        auto shutdown_json = json::parse(shutdown_res->body);
        CHECK(shutdown_json["status"] == "ok");
        std::cout << "关闭请求已发送" << std::endl;
        
        // 异步关闭服务器
        auto stop_future = stopServerAsync(server);
        
        // 等待服务器关闭
        std::cout << "等待服务器关闭..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 验证服务器已停止
        CHECK_FALSE(server.isRunning());
        std::cout << "服务器已停止" << std::endl;
        
        // 尝试再次访问健康检查API，应该失败
        std::cout << "验证服务器已完全关闭..." << std::endl;
        auto health_res = cli.Get("/api/health");
        CHECK(health_res == nullptr);
        
        // 等待关闭操作完成
        std::cout << "等待关闭操作完成..." << std::endl;
        stop_future.wait();
        std::cout << "关闭操作完成" << std::endl;
    }
    
    std::cout << "=== 基本服务器功能测试完成 ===\n" << std::endl;
}

// 测试用例2：模拟控制API测试
TEST_CASE("simulation_control_api_test") {
    std::cout << "\n=== 开始模拟控制API测试 ===\n";
    
    // 创建服务器实例
    sim::Server server(8080);
    std::cout << "服务器实例已创建\n";
    
    // 异步启动服务器
    std::cout << "开始异步启动服务器...\n";
    auto serverThread = startServerAsync(server);
    std::cout << "服务器异步启动成功\n";
    
    // 等待服务器启动
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 创建HTTP客户端
    httplib::Client cli("http://localhost:8080");
    std::cout << "HTTP客户端已创建\n";
    
    // 创建生产者-消费者网络
    auto createRes = cli.Post("/api/network/create/producer-consumer");
    CHECK(createRes);
    CHECK(createRes->status == 200);
    std::cout << "成功创建生产者-消费者网络\n";
    
    // 启动模拟
    auto start_res = cli.Post("/api/simulation/start");
    CHECK(start_res->status == 200);
    CHECK(start_res->body == "{\"status\":\"ok\"}");
    
    // 等待模拟完全启动
    std::cout << "等待模拟启动..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // 检查状态
    auto state_res = cli.Get("/api/network/state");
    CHECK(state_res->status == 200);
    
    // 打印原始响应内容
    std::cout << "状态响应内容: " << state_res->body << std::endl;
    
    // 解析JSON
    json state_json;
    try {
        std::string raw_body = state_res->body;
        
        // 处理可能被引号包裹的JSON字符串
        if (raw_body.front() == '"' && raw_body.back() == '"') {
            // 移除外部引号
            raw_body = raw_body.substr(1, raw_body.length() - 2);
            
            // 处理转义字符
            std::string::size_type pos = 0;
            while ((pos = raw_body.find("\\\"", pos)) != std::string::npos) {
                raw_body.replace(pos, 2, "\"");
                pos += 1;
            }
            
            // 处理其他转义字符
            pos = 0;
            while ((pos = raw_body.find("\\\\", pos)) != std::string::npos) {
                raw_body.replace(pos, 2, "\\");
                pos += 1;
            }
        }
        
        std::cout << "处理后的JSON字符串: " << raw_body << std::endl;
        
        // 解析处理后的JSON
        state_json = json::parse(raw_body);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON解析错误: " << e.what() << std::endl;
        CHECK(false);
    }
    
    // 打印解析后的JSON类型和内容
    std::cout << "解析后的JSON类型: " << state_json.type_name() << std::endl;
    std::cout << "解析后的JSON内容: " << state_json.dump(2) << std::endl;
    
    // 检查状态
    CHECK(state_json.is_object());
    CHECK(state_json["running"].is_boolean());
    CHECK(state_json["running"] == true);
    
    // 停止模拟
    auto stop_res = cli.Post("/api/simulation/stop");
    CHECK(stop_res->status == 200);
    CHECK(stop_res->body == "{\"status\":\"ok\"}");
    
    // 等待模拟完全停止
    std::cout << "等待模拟停止..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // 再次检查状态
    state_res = cli.Get("/api/network/state");
    CHECK(state_res->status == 200);
    
    // 解析JSON
    try {
        std::string raw_body = state_res->body;
        
        // 处理可能被引号包裹的JSON字符串
        if (raw_body.front() == '"' && raw_body.back() == '"') {
            // 移除外部引号
            raw_body = raw_body.substr(1, raw_body.length() - 2);
            
            // 处理转义字符
            std::string::size_type pos = 0;
            while ((pos = raw_body.find("\\\"", pos)) != std::string::npos) {
                raw_body.replace(pos, 2, "\"");
                pos += 1;
            }
            
            // 处理其他转义字符
            pos = 0;
            while ((pos = raw_body.find("\\\\", pos)) != std::string::npos) {
                raw_body.replace(pos, 2, "\\");
                pos += 1;
            }
        }
        
        std::cout << "处理后的JSON字符串: " << raw_body << std::endl;
        
        // 解析处理后的JSON
        state_json = json::parse(raw_body);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON解析错误: " << e.what() << std::endl;
        CHECK(false);
    }
    
    // 检查状态
    CHECK(state_json.is_object());
    CHECK(state_json["running"].is_boolean());
    CHECK(state_json["running"] == false);
    
    // 清理资源
    std::cout << "清理服务器资源...\n";
    if (server.isRunning()) {
        std::cout << "开始异步关闭服务器...\n";
        auto shutdownRes = cli.Post("/api/server/shutdown");
        CHECK(shutdownRes);
        CHECK(shutdownRes->status == 200);
        
        // 等待服务器关闭
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // 显式调用服务器的stop方法
        server.stop();
        std::cout << "服务器已停止\n";
    }
    
    // 确保完全关闭(防止SIGABRT崩溃)
    try {
        std::cout << "确保服务器已彻底关闭...\n";
        // 在退出前再次确认服务器已停止
        CHECK_FALSE(server.isRunning());
        std::cout << "服务器状态确认为已停止\n";
    } catch (...) {
        std::cout << "捕获到异常，忽略并继续...\n";
    }
    
    std::cout << "服务器异步关闭完成\n";
    std::cout << "服务器资源已清理\n";
    std::cout << "=== 模拟控制API测试完成 ===\n";
} 