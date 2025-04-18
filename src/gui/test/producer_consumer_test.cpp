#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include "gui_test_framework.h"
#include "network_viewer.h"
#include "http_client.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <imgui.h>
#include <nlohmann/json.hpp>

// 添加sim相关头文件
#include "sim/include/producer_consumer.h"

// 手动包含server.h中的sim命名空间内容
namespace sim {
    class Server {
    public:
        Server(int port);
        virtual ~Server();
        bool start();
        void stop();
        bool isRunning() const;
        bool loadNetworkFromFile(const std::string& filepath);
        bool loadNetworkFromJson(const std::string& jsonStr);
        nlohmann::json getNetworkState() const;
        bool startSimulation();
        bool stopSimulation();
        bool stepSimulation();
        bool resetSimulation();
        void setStateUpdateCallback(std::function<void(const nlohmann::json&)> callback);
        bool createProducerConsumerNetwork();
    };
}

// 全局变量，控制是否运行可视化测试
bool g_RunVisualTest = true;  // 默认开启可视化

// 是否使用模拟UI（不实际渲染NetworkViewer）
bool g_UseMockUI = false;  // 默认使用真实UI

// 测试用例函数签名
void test_producer_consumer_network(int argc, char** argv);

// 测试夹具类
class ProducerConsumerFixture {
public:
    ProducerConsumerFixture() 
        : viewerInitialized(false)
        , server_running(false)
        , use_external_server(false)
        , server_url("http://localhost:8080") 
    {
        // 初始化框架
        if (!framework.init()) {
            std::cerr << "无法初始化GUI测试框架" << std::endl;
            return;
        }
        
        // 初始化NetworkViewer
        if (viewer.init()) {
            viewerInitialized = true;
            framework.setNetworkViewer(&viewer);
        } else {
            std::cerr << "NetworkViewer初始化失败" << std::endl;
            viewerInitialized = false;
        }
    }
    
    ~ProducerConsumerFixture() {
        // 断开连接
        viewer.disconnectFromServer();
        
        // 停止服务器
        server_running = false;
        if (server_thread.joinable()) {
            server_thread.join();
        }
        
        // 清理资源
        viewer.shutdown();
        framework.shutdown();
    }
    
    void parseArgs(int argc, char** argv) {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--external-server" && i + 1 < argc) {
                use_external_server = true;
                server_url = argv[i + 1];
                i++;
                std::cout << "将使用外部服务器: " << server_url << std::endl;
            }
        }
    }
    
    // 启动服务器方法
    void startServerThread() {
        // 如果使用外部服务器，则不启动本地服务器
        if (use_external_server) {
            std::cout << "使用外部服务器: " << server_url << std::endl;
            server_running = true;
            return;
        }
        
        // 启动内部服务器线程
        server_running = true;
        server_thread = std::thread([this]() {
            try {
                // 创建并启动服务器
                std::cout << "启动本地服务器..." << std::endl;
                
                #ifdef USE_MOCK_SERVER
                MockServer server(8080);
                #else
                sim::Server server(8080);
                #endif
                
                // 启动服务器
                if (!server.start()) {
                    std::cerr << "无法启动服务器" << std::endl;
                    server_running = false;
                    return;
                }
                
                std::cout << "服务器已启动，运行在端口 8080" << std::endl;
                
                // 创建生产者-消费者网络
                bool networkCreated = false;
                try {
                    // 在创建网络前先等待一段时间，确保服务器已完全启动
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    
                    networkCreated = server.createProducerConsumerNetwork();
                    if (networkCreated) {
                        std::cout << "成功创建生产者-消费者网络" << std::endl;
                    } else {
                        std::cerr << "无法创建生产者-消费者网络" << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "创建网络时发生异常: " << e.what() << std::endl;
                    networkCreated = false;
                }
                
                if (!networkCreated) {
                    std::cerr << "无法创建生产者-消费者网络" << std::endl;
                    server.stop();
                    server_running = false;
                    return;
                }
                
                std::cout << "已创建生产者-消费者网络" << std::endl;
                
                // 设置状态更新回调
                server.setStateUpdateCallback([](const nlohmann::json& state) {
                    std::cout << "网络状态已更新: " << state.dump().substr(0, 100) << "..." << std::endl;
                });
                
                // 服务器主循环 - 处理请求直到收到停止信号
                int tick = 0;
                while (server_running) {
                    // 稍微暂停以减少CPU使用
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                
                // 停止服务器
                std::cout << "正在停止服务器..." << std::endl;
                server.stop();
                std::cout << "服务器已停止" << std::endl;
                
            } catch (const std::exception& e) {
                std::cerr << "服务器线程发生异常: " << e.what() << std::endl;
                server_running = false;
            } catch (...) {
                std::cerr << "服务器线程发生未知异常" << std::endl;
                server_running = false;
            }
            
            std::cout << "服务器线程已退出" << std::endl;
        });
    }

    // 添加可视化显示方法
    void showVisualForSeconds(float seconds, const std::function<void(int)>& updateFunc = nullptr) {
        if (!g_RunVisualTest) {
            // 在非可视化模式下，仍然尝试执行更新函数
            if (updateFunc) {
                try {
                    updateFunc(0);
                } catch (const std::exception& e) {
                    std::cerr << "执行更新函数时发生异常: " << e.what() << std::endl;
                }
            }
            return;
        }
        
        try {
            int frameCount = static_cast<int>(seconds * 10); // 每秒显示10帧
            bool updateCalled = false; // 跟踪更新函数是否已调用
            
            for (int i = 0; i < frameCount; i++) {
                // 如果提供了更新函数，则在第一帧调用它
                if (updateFunc && !updateCalled && i == 0) {
                    try {
                        updateFunc(i);
                        updateCalled = true; // 标记为已调用
                    } catch (const std::exception& e) {
                        std::cerr << "执行更新函数时发生异常: " << e.what() << std::endl;
                    }
                }
                
                // 执行渲染逻辑
                framework.runOneFrame();
                
                // 适当延迟，减少CPU使用
                std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 减少等待时间
            }
        } catch (const std::exception& e) {
            std::cerr << "显示可视化界面时发生异常: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "显示可视化界面时发生未知异常" << std::endl;
        }
    }

    // 安全访问网络查看器
    template<typename Func>
    void safeViewerAccess(Func func, int maxRetries = 1) {
        int retryCount = 0;
        bool success = false;
        
        while (!success && retryCount < maxRetries) {
            try {
                // 执行函数
                func();
                success = true;
            } catch (const std::exception& e) {
                retryCount++;
                std::cerr << "访问NetworkViewer时发生异常 (尝试 " << retryCount << "/" << maxRetries << "): " 
                          << e.what() << std::endl;
                
                if (retryCount >= maxRetries) {
                    std::cerr << "已达到最大重试次数，放弃操作" << std::endl;
                    break;
                }
                
                // 等待一段时间后重试
                std::this_thread::sleep_for(std::chrono::milliseconds(100 * retryCount));
            } catch (...) {
                retryCount++;
                std::cerr << "访问NetworkViewer时发生未知异常 (尝试 " << retryCount << "/" << maxRetries << ")" << std::endl;
                
                if (retryCount >= maxRetries) {
                    std::cerr << "已达到最大重试次数，放弃操作" << std::endl;
                    break;
                }
                
                // 等待一段时间后重试
                std::this_thread::sleep_for(std::chrono::milliseconds(100 * retryCount));
            }
        }
    }
    
    // 创建JSON形式的网络状态
    std::string createNetworkStateJson() {
        // 创建一个生产者-消费者网络的JSON表示，用于GUI可视化
        nlohmann::json networkState;
        networkState["status"] = "success";
        networkState["data"]["tick"] = 0;
        networkState["data"]["running"] = false;
        
        // 生产者节点
        nlohmann::json producer;
        producer["id"] = 1;
        producer["type"] = "producer";
        producer["name"] = "生产者节点";
        producer["properties"]["produced_count"] = 0;
        producer["position"]["x"] = 100;
        producer["position"]["y"] = 100;
        producer["inputs"] = nlohmann::json::array();
        
        nlohmann::json producerOutput;
        producerOutput["id"] = 1;
        producerOutput["name"] = "out";
        producerOutput["type"] = "void";
        producerOutput["connected"] = true;
        producer["outputs"] = nlohmann::json::array({producerOutput});
        
        // 消费者节点
        nlohmann::json consumer;
        consumer["id"] = 2;
        consumer["type"] = "consumer";
        consumer["name"] = "消费者节点";
        consumer["properties"]["consumed_count"] = 0;
        consumer["position"]["x"] = 400;
        consumer["position"]["y"] = 100;
        
        nlohmann::json consumerInput;
        consumerInput["id"] = 2;
        consumerInput["name"] = "in";
        consumerInput["type"] = "void";
        consumerInput["connected"] = true;
        consumer["inputs"] = nlohmann::json::array({consumerInput});
        consumer["outputs"] = nlohmann::json::array();
        
        // 连接
        nlohmann::json connection;
        connection["id"] = 1;
        connection["from_node"] = 1;
        connection["from_port"] = 1;
        connection["to_node"] = 2;
        connection["to_port"] = 2;
        
        networkState["data"]["nodes"] = nlohmann::json::array({producer, consumer});
        networkState["data"]["connections"] = nlohmann::json::array({connection});
        
        return networkState.dump();
    }

    GuiTestFramework framework;
    NetworkViewer viewer;
    bool viewerInitialized;
    
    // 服务器线程控制
    std::thread server_thread;
    std::atomic<bool> server_running;
    bool use_external_server;
    std::string server_url;
};

// 修改测试用例定义
TEST_CASE_FIXTURE(ProducerConsumerFixture, "生产者-消费者网络测试") {
}

// 实际测试实现
void test_producer_consumer_network(ProducerConsumerFixture& fixture, int argc, char** argv) {
    // 启动服务器
    MESSAGE("启动服务器");
    fixture.startServerThread();
    
    // 等待服务器启动完成
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    if (!g_RunVisualTest) {
        MESSAGE("运行自动测试...");
        
        // 如果服务器正在运行，尝试连接
        CHECK(fixture.server_running);
        CHECK(fixture.viewer.connectToServer(fixture.server_url));
        
        // 等待连接建立
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        // 执行模拟测试
        // 测试单步执行
        fixture.safeViewerAccess([&fixture]() {
            MESSAGE("测试单步执行...");
            CHECK_FALSE(fixture.viewer.isSimulationRunning());
            bool success = fixture.viewer.stepSimulation();
            CHECK(success);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }, 3);
        
        // 测试持续运行
        fixture.safeViewerAccess([&fixture]() {
            MESSAGE("测试启动模拟...");
            bool success = fixture.viewer.startSimulation();
            CHECK(success);
            CHECK(fixture.viewer.isSimulationRunning());
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }, 3);
        
        // 测试停止
        fixture.safeViewerAccess([&fixture]() {
            MESSAGE("测试停止模拟...");
            bool success = fixture.viewer.stopSimulation();
            CHECK(success);
            CHECK_FALSE(fixture.viewer.isSimulationRunning());
        }, 3);
        
        // 测试重置
        fixture.safeViewerAccess([&fixture]() {
            MESSAGE("测试重置模拟...");
            bool success = fixture.viewer.resetSimulation();
            CHECK(success);
            CHECK_EQ(fixture.viewer.getCurrentTick(), 0);
        }, 3);
    } else {
        // 可视化测试
        MESSAGE("运行可视化测试，请使用UI操作模拟器...");
        
        // 等待用户交互
        bool shouldContinue = true;
        while (shouldContinue && fixture.server_running) {
            // 运行一帧GUI
            try {
                fixture.framework.runOneFrame();
                // 检查是否应该继续 - 可以基于其他条件
                shouldContinue = !ImGui::IsKeyPressed(ImGuiKey_Escape); // 按Esc退出
            } catch (const std::exception& e) {
                std::cerr << "GUI渲染出错: " << e.what() << std::endl;
                shouldContinue = false;
            }
        }
    }
}

// 主函数 - 负责处理命令行参数并运行测试
int main(int argc, char** argv) {
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--visual") {
            g_RunVisualTest = true;
        } else if (arg == "--mock-ui") {
            g_UseMockUI = true;
        }
    }

    std::cout << "======== 生产者-消费者网络测试 ========\n"
              << "模式: " << (g_RunVisualTest ? "可视化" : "自动") << ", "
              << "UI: " << (g_UseMockUI ? "模拟" : "真实") << std::endl;

    // 创建测试实例并解析命令行参数
    ProducerConsumerFixture fixture;
    fixture.parseArgs(argc, argv);
    
    // 运行测试
    test_producer_consumer_network(fixture, argc, argv);
    
    return 0;
} 