#ifndef NETWORK_VIEWER_HPP
#define NETWORK_VIEWER_HPP

#include "imgui.h"
#include "network_map.h"
#include <string>
#include <functional>

// 定义视图模式的枚举
enum class ViewMode {
    NetworkMap,    // 网络地图视图
    Timeline,      // 时间线视图
    TrafficChart   // 流量图表视图
};

class NetworkViewer {
public:
    void render(); // 渲染主界面
    ViewMode getCurrentMode() const; // 获取当前视图模式
    bool shouldQuit() const; // 检查是否请求退出

private:
    ViewMode currentMode = ViewMode::NetworkMap; // 当前视图模式，默认为网络地图
    bool showQuitConfirmation = false; // 是否显示退出确认对话框
    bool quitRequested = false; // 是否请求退出
    NetworkMap networkMap; // Add an instance of NetworkMap

    struct WindowState {
        ImVec2 previousSize = ImVec2(800, 600); // 上一次窗口大小
        ImVec2 previousPos = ImVec2(100, 100); // 上一次窗口位置
        bool isMaximized = false; // 是否最大化
    };

    WindowState networkMapState; // 网络地图窗口状态
    WindowState timelineState;   // 时间线窗口状态
    WindowState trafficChartState; // 流量图表窗口状态

    void renderMenuBar(); // 渲染菜单栏
    void resetWindowSizes(); // 重置所有窗口大小
    void renderQuitConfirmationDialog(); // 渲染退出确认对话框
    void renderNetworkMap(); // 渲染网络地图视图
    void renderTimeline(); // 渲染时间线视图
    void renderTrafficChart(); // 渲染流量图表视图

    void handleWindowState(const char* title, WindowState& state, ViewMode mode, const std::function<void()>& renderContent); // 处理窗口状态
    ImGuiWindowFlags getWindowFlags(ViewMode mode) const; // 获取窗口标志
    void LoadNetworkFromFile(const std::string& filePath); // 从文件加载网络数据
};

#endif // NETWORK_VIEWER_HPP
