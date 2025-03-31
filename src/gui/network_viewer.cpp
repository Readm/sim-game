#include "imgui.h"
#include <vector>
#include <string>
#include <functional> // 添加此行以使用 std::function
#include <iostream> // 添加此行以进行调试输出

// 定义视图模式的枚举
enum class ViewMode {
    NetworkMap,    // 网络地图视图
    Timeline,      // 时间线视图
    TrafficChart   // 流量图表视图
};


// TODO_BUGFIX: 点击窗口无法提到最前
// TODO_BUGFIX: 再次点击一个窗口隐藏窗口

class NetworkViewer {
public:
    // 渲染主界面
    void render() {
        renderMenuBar(); // 渲染菜单栏
        if (showQuitConfirmation) {
            renderQuitConfirmationDialog(); // 渲染退出确认对话框
        }
        renderNetworkMap(); // 渲染网络地图视图
        renderTimeline();   // 渲染时间线视图
        renderTrafficChart(); // 渲染流量图表视图
    }

    // 获取当前视图模式
    ViewMode getCurrentMode() const {
        return currentMode;
    }

    // 检查是否请求退出
    bool shouldQuit() const {
        return quitRequested;
    }

private:
    ViewMode currentMode = ViewMode::NetworkMap; // 当前视图模式，默认为网络地图
    bool showQuitConfirmation = false; // 是否显示退出确认对话框
    bool quitRequested = false; // 是否请求退出

    // 窗口状态结构体
    struct WindowState {
        ImVec2 previousSize = ImVec2(800, 600); // 上一次窗口大小
        ImVec2 previousPos = ImVec2(100, 100); // 上一次窗口位置
        bool isMaximized = false; // 是否最大化
    };

    WindowState networkMapState; // 网络地图窗口状态
    WindowState timelineState;   // 时间线窗口状态
    WindowState trafficChartState; // 流量图表窗口状态

    // 渲染菜单栏
    void renderMenuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("View")) {
                // 切换到网络地图视图
                if (ImGui::MenuItem("Network Map", nullptr, currentMode == ViewMode::NetworkMap)) {
                    currentMode = ViewMode::NetworkMap;
                }
                // 切换到时间线视图
                if (ImGui::MenuItem("Timeline", nullptr, currentMode == ViewMode::Timeline)) {
                    currentMode = ViewMode::Timeline;
                }
                // 切换到流量图表视图
                if (ImGui::MenuItem("Traffic Chart", nullptr, currentMode == ViewMode::TrafficChart)) {
                    currentMode = ViewMode::TrafficChart;
                }
                ImGui::Separator(); // 添加分隔符
                // 重置窗口大小
                if (ImGui::MenuItem("Reset Window Sizes")) {
                    std::cout << "Reset Window Sizes button clicked" << std::endl; // 调试日志
                    resetWindowSizes(); // 调用重置函数
                }
                ImGui::EndMenu();
            }
            // 显示退出选项
            if (ImGui::MenuItem("Quit")) {
                showQuitConfirmation = true;
            }
            ImGui::EndMainMenuBar();
        }
    }

    // 重置所有窗口大小
    void resetWindowSizes() {
        std::cout << "Resetting window sizes..." << std::endl; // 调试日志

        // 重置网络地图窗口
        networkMapState.isMaximized = false;
        ImGui::SetWindowSize("Network Map", ImVec2(800, 600));
        ImGui::SetWindowPos("Network Map", ImVec2(100, 100));

        // 重置时间线窗口
        timelineState.isMaximized = false;
        ImGui::SetWindowSize("Timeline", ImVec2(800, 600));
        ImGui::SetWindowPos("Timeline", ImVec2(100, 100));

        // 重置流量图表窗口
        trafficChartState.isMaximized = false;
        ImGui::SetWindowSize("Traffic Chart", ImVec2(800, 600));
        ImGui::SetWindowPos("Traffic Chart", ImVec2(100, 100));

        std::cout << "Window sizes reset successfully." << std::endl; // 调试日志
    }

    // 渲染退出确认对话框
    void renderQuitConfirmationDialog() {
        ImGui::OpenPopup("Confirm Quit");
        if (ImGui::BeginPopupModal("Confirm Quit", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Are you sure you want to quit?");
            if (ImGui::Button("Yes")) {
                quitRequested = true;
                showQuitConfirmation = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("No")) {
                showQuitConfirmation = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    // 渲染网络地图视图
    void renderNetworkMap() {
        handleWindowState("Network Map", networkMapState, ViewMode::NetworkMap, []() {
            ImGui::Text("Network Map View");
            // 在此处添加渲染网络地图的逻辑
        });
    }

    // 渲染时间线视图
    void renderTimeline() {
        handleWindowState("Timeline", timelineState, ViewMode::Timeline, []() {
            ImGui::Text("Timeline View");
            // 在此处添加渲染时间线的逻辑
        });
    }

    // 渲染流量图表视图
    void renderTrafficChart() {
        handleWindowState("Traffic Chart", trafficChartState, ViewMode::TrafficChart, []() {
            ImGui::Text("Traffic Chart View");
            // 在此处添加渲染流量图表的逻辑
        });
    }

    // 处理窗口状态
    void handleWindowState(const char* title, WindowState& state, ViewMode mode, const std::function<void()>& renderContent) {
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

        if (state.isMaximized) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            float menuBarHeight = ImGui::GetFrameHeight(); // 菜单栏高度
            ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menuBarHeight));
            ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight));
        }

        if (ImGui::Begin(title, nullptr, getWindowFlags(mode))) {
            // 添加最大化/还原按钮
            ImGui::SameLine(ImGui::GetWindowWidth() - 120);
            if (ImGui::SmallButton(state.isMaximized ? "[Restore]" : "[Maximize]")) {
                if (!state.isMaximized) {
                    state.previousSize = ImGui::GetWindowSize();
                    state.previousPos = ImGui::GetWindowPos();
                    ImGuiViewport* viewport = ImGui::GetMainViewport();
                    float menuBarHeight = ImGui::GetFrameHeight();
                    ImGui::SetWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menuBarHeight));
                    ImGui::SetWindowPos(ImVec2(0, menuBarHeight));
                } else {
                    ImGui::SetWindowSize(state.previousSize);
                    ImGui::SetWindowPos(state.previousPos);
                }
                state.isMaximized = !state.isMaximized;
            }

            renderContent();
        }
        ImGui::End();
    }

    // 获取窗口标志
    ImGuiWindowFlags getWindowFlags(ViewMode mode) const {
        // 仅对非活动视图应用 NoBringToFrontOnFocus 标志
        return (currentMode == mode) ? 0 : ImGuiWindowFlags_NoBringToFrontOnFocus;
    }
};
