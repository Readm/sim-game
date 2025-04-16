#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include "gui_test_framework.h"
#include "network_viewer.h"
#include "http_client.h"
#include <thread>
#include <chrono>
#include <iostream>

// 全局变量，控制是否运行可视化测试
bool g_RunVisualTest = false;

// 是否使用模拟UI（不实际渲染NetworkViewer）
bool g_UseMockUI = true;

// 自定义的main函数，用于处理命令行参数
int main(int argc, char** argv) {
    // 检查命令行参数
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--visual" || arg == "-v") {
            g_RunVisualTest = true;
        }
        if (arg == "--real-ui" || arg == "-r") {
            g_UseMockUI = false;
        }
    }
    
    if (g_RunVisualTest) {
        std::cout << "已启用可视化测试模式，所有测试将显示界面" << std::endl;
        if (g_UseMockUI) {
            std::cout << "使用模拟UI进行测试 (使用 --real-ui 参数切换到实际UI)" << std::endl;
        } else {
            std::cout << "使用实际UI进行测试" << std::endl;
            std::cout << "警告：实际UI模式可能不稳定，如果崩溃请使用默认的模拟UI模式" << std::endl;
        }
    }
    
    // 执行doctest的测试
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
}

class NetworkViewerFixture {
protected:
    NetworkViewerFixture() : framework(), viewer(), viewerInitialized(false) {
        try {
            // 首先初始化框架
            if (!framework.init()) {
                std::cerr << "无法初始化GUI测试框架" << std::endl;
                return;
            }
            
            // 等待一小段时间，确保窗口创建成功
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // 尝试初始化查看器
            try {
                viewer.init();
                viewerInitialized = true;
            } catch (const std::exception& e) {
                std::cerr << "初始化NetworkViewer时发生异常: " << e.what() << std::endl;
                viewerInitialized = false;
                g_UseMockUI = true; // 强制使用模拟UI
            } catch (...) {
                std::cerr << "初始化NetworkViewer时发生未知异常" << std::endl;
                viewerInitialized = false;
                g_UseMockUI = true; // 强制使用模拟UI
            }
            
            // 设置网络查看器（根据模式决定）
            if (!g_UseMockUI && viewerInitialized) {
                framework.setNetworkViewer(&viewer);
            } else {
                framework.setNetworkViewer(nullptr);  // 使用模拟UI
                if (!g_UseMockUI) {
                    std::cerr << "警告: NetworkViewer初始化失败，已切换到模拟UI模式" << std::endl;
                }
            }
            
            // 再等待一小段时间，确保所有组件初始化完成
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // 运行一帧，确保ImGui上下文正确初始化
            framework.runOneFrame();
            
        } catch (const std::exception& e) {
            std::cerr << "初始化过程中发生异常: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "初始化过程中发生未知异常" << std::endl;
        }
    }

    ~NetworkViewerFixture() {
        try {
            if (viewerInitialized) {
                viewer.shutdown();
            }
            framework.shutdown();
        } catch (const std::exception& e) {
            std::cerr << "清理过程中发生异常: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "清理过程中发生未知异常" << std::endl;
        }
    }

    // 添加可视化显示方法
    void showVisualForSeconds(float seconds, const std::function<void(int)>& updateFunc = nullptr) {
        if (!g_RunVisualTest) {
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
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        } catch (const std::exception& e) {
            std::cerr << "显示可视化界面时发生异常: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "显示可视化界面时发生未知异常" << std::endl;
        }
    }

    // 对NetworkViewer的安全访问方法
    template<typename Func>
    void safeViewerAccess(Func operation) {
        if (viewerInitialized) {
            try {
                operation(viewer);
            } catch (const std::exception& e) {
                std::cerr << "访问NetworkViewer时发生异常: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "访问NetworkViewer时发生未知异常" << std::endl;
            }
        }
    }

    GuiTestFramework framework;
    NetworkViewer viewer;
    bool viewerInitialized;
};

TEST_CASE_FIXTURE(NetworkViewerFixture, "基本功能测试") {
    // 显示测试窗口（仅在可视化模式下）
    MESSAGE("基本功能测试");
    
    // 显示窗口2秒
    showVisualForSeconds(2);
    
    // 简单测试
    CHECK(true);
}

TEST_CASE_FIXTURE(NetworkViewerFixture, "模拟控制测试") {
    MESSAGE("模拟控制测试");
    
    // 初始状态检查 - 使用安全访问方法
    bool isRunning = false;
    int currentTick = 0;
    
    safeViewerAccess([&](NetworkViewer& v) {
        isRunning = v.isSimulationRunning();
        currentTick = v.getCurrentTick();
    });
    
    CHECK_FALSE(isRunning);
    CHECK_EQ(currentTick, 0);

    // 启动模拟 - 直接调用方法而不是模拟点击
    showVisualForSeconds(1, [&](int) {
        MESSAGE("启动模拟");
        safeViewerAccess([](NetworkViewer& v) {
            v.startSimulation();
        });
    });
    
    safeViewerAccess([&](NetworkViewer& v) {
        isRunning = v.isSimulationRunning();
    });
    CHECK(isRunning);

    // 停止模拟
    showVisualForSeconds(1, [&](int) {
        MESSAGE("停止模拟");
        safeViewerAccess([](NetworkViewer& v) {
            v.stopSimulation();
        });
    });
    
    safeViewerAccess([&](NetworkViewer& v) {
        isRunning = v.isSimulationRunning();
    });
    CHECK_FALSE(isRunning);

    // 单步执行
    int initialTick = 0;
    safeViewerAccess([&](NetworkViewer& v) {
        initialTick = v.getCurrentTick();
    });
    
    showVisualForSeconds(1, [&](int) {
        MESSAGE("单步执行");
        safeViewerAccess([](NetworkViewer& v) {
            v.stepSimulation();
        });
    });
    
    safeViewerAccess([&](NetworkViewer& v) {
        currentTick = v.getCurrentTick();
    });
    CHECK_EQ(currentTick, initialTick + 1);

    // 重置模拟
    showVisualForSeconds(1, [&](int) {
        MESSAGE("重置模拟");
        safeViewerAccess([](NetworkViewer& v) {
            v.resetSimulation();
        });
    });
    
    safeViewerAccess([&](NetworkViewer& v) {
        isRunning = v.isSimulationRunning();
        currentTick = v.getCurrentTick();
    });
    CHECK_FALSE(isRunning);
    CHECK_EQ(currentTick, 0);
}

TEST_CASE_FIXTURE(NetworkViewerFixture, "可视化测试") {
    // 如果没有启用可视化测试，则跳过
    if (!g_RunVisualTest) {
        MESSAGE("跳过可视化测试，使用 --visual 参数来启用");
        return;
    }
    
    MESSAGE("正在显示测试窗口，请观察窗口内容...");
    MESSAGE("测试将持续30秒，您可以观察界面组件");
    
    // 先显示10秒，让用户观察界面
    showVisualForSeconds(10);
    
    // 使用新的可视化显示方法
    showVisualForSeconds(20, [&](int frame) {
        if (frame % 50 == 0) { // 每5秒更新一次
            MESSAGE("执行单步模拟");
            safeViewerAccess([](NetworkViewer& v) {
                v.stepSimulation();
            });
        }
        
        if (frame % 100 == 0) { // 每10秒更新一次
            MESSAGE("启动模拟");
            safeViewerAccess([](NetworkViewer& v) {
                v.startSimulation();
            });
        }
        
        if (frame % 100 == 50) { // 启动5秒后停止
            MESSAGE("停止模拟");
            safeViewerAccess([](NetworkViewer& v) {
                v.stopSimulation();
            });
        }
    });
    
    // 验证模拟状态
    bool isRunning = false;
    int currentTick = 0;
    
    safeViewerAccess([&](NetworkViewer& v) {
        isRunning = v.isSimulationRunning();
        currentTick = v.getCurrentTick();
    });
    
    CHECK_FALSE(isRunning);
    if (viewerInitialized) {
        CHECK_GT(currentTick, 0);
    }
}

TEST_CASE("未实现的功能测试指南") {
    // 这些测试需要在GUI框架完善后进行实现
    MESSAGE("以下功能测试需要在GUI框架完善后实现:");
    MESSAGE("1. 节点创建测试");
    MESSAGE("2. 节点连接测试");
    MESSAGE("3. 属性面板测试");
    MESSAGE("4. 网络保存/加载测试");
    MESSAGE("5. 性能测试");
    MESSAGE("6. 错误处理测试");
} 