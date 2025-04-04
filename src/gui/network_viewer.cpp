#include "network_viewer.hpp"
#include <iostream> // 添加此行以进行调试输出
#include <fstream>
#include <ImGuiFileDialog.h> // Include a file dialog library like ImGuiFileDialog
#include <nlohmann/json.hpp>
#include <filesystem> // For file operations

// Add a member variable to store the last file path
std::string lastFilePath;

// Add a constant for the file path to store the last session
const std::string LAST_SESSION_FILE = "last_session.txt";

// Modify the constructor to load the last file path
NetworkViewer::NetworkViewer() {
    if (std::filesystem::exists(LAST_SESSION_FILE)) {
        std::ifstream file(LAST_SESSION_FILE);
        if (file.is_open()) {
            std::getline(file, lastFilePath);
            file.close();
        }
    }
}

// Modify the destructor to save the last file path
NetworkViewer::~NetworkViewer() {
    if (!lastFilePath.empty()) {
        std::ofstream file(LAST_SESSION_FILE);
        if (file.is_open()) {
            file << lastFilePath;
            file.close();
        }
    }
}

// 渲染主界面
void NetworkViewer::render() {
    renderMenuBar(); // 渲染菜单栏
    if (showQuitConfirmation) {
        renderQuitConfirmationDialog(); // 渲染退出确认对话框
    }
    renderNetworkMap(); // 渲染网络地图视图
    renderTimeline();   // 渲染时间线视图
    renderTrafficChart(); // 渲染流量图表视图

    // 处理加载文件对话框
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            lastFilePath = filePath; // Save the file path for restoring later
            LoadNetworkFromFile(filePath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    // 处理保存文件对话框
    if (ImGuiFileDialog::Instance()->Display("SaveFileDlgKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            SaveNetworkToFile(filePath);
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

// 获取当前视图模式
ViewMode NetworkViewer::getCurrentMode() const {
    return currentMode;
}

// 检查是否请求退出
bool NetworkViewer::shouldQuit() const {
    return quitRequested;
}

// 渲染菜单栏
void NetworkViewer::renderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            // Add "Restore Last Session" button
            if (ImGui::MenuItem("Restore Last Session", nullptr, !lastFilePath.empty())) {
                if (!lastFilePath.empty()) {
                    LoadNetworkFromFile(lastFilePath);
                }
            }
            if (ImGui::MenuItem("Load Network")) {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose JSON File", ".json", ".");
            }
            if (ImGui::MenuItem("Save Network As...")) {
                ImGuiFileDialog::Instance()->OpenDialog("SaveFileDlgKey", "Save JSON File", ".json", ".");
            }
            ImGui::EndMenu();
        }
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

        // Move "Quit" button to the far right
        ImGui::SameLine(ImGui::GetWindowWidth() - 50); // Adjust the offset as needed
        if (ImGui::MenuItem("Quit")) {
            showQuitConfirmation = true;
        }
        ImGui::EndMainMenuBar();
    }
}

// 重置所有窗口大小
void NetworkViewer::resetWindowSizes() {
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
void NetworkViewer::renderQuitConfirmationDialog() {
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
void NetworkViewer::renderNetworkMap() {
    handleWindowState("Network Map", networkMapState, ViewMode::NetworkMap, [this]() {
        // 在渲染网络地图之前检查是否有节点正在被拖拽
        networkMap.render();
    });
}

// 渲染时间线视图
void NetworkViewer::renderTimeline() {
    handleWindowState("Timeline", timelineState, ViewMode::Timeline, []() {
        ImGui::Text("Timeline View");
        // 在此处添加渲染时间线的逻辑
    });
}

// 渲染流量图表视图
void NetworkViewer::renderTrafficChart() {
    handleWindowState("Traffic Chart", trafficChartState, ViewMode::TrafficChart, []() {
        ImGui::Text("Traffic Chart View");
        // 在此处添加渲染流量图表的逻辑
    });
}

// 处理窗口状态
void NetworkViewer::handleWindowState(const char* title, WindowState& state, ViewMode mode, const std::function<void()>& renderContent) {
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

    if (state.isMaximized) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        float menuBarHeight = ImGui::GetFrameHeight();
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menuBarHeight));
        ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight));
    }

    // 设置窗口标志
    ImGuiWindowFlags flags = getWindowFlags(mode);
    
    // 如果是网络地图窗口且有节点正在拖拽，添加 NoMove 标志
    if (mode == ViewMode::NetworkMap && networkMap.isAnyNodeDragging()) {
        flags |= ImGuiWindowFlags_NoMove;
    }

    if (ImGui::Begin(title, nullptr, flags)) {
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
ImGuiWindowFlags NetworkViewer::getWindowFlags(ViewMode mode) const {
    // 仅对非活动视图应用 NoBringToFrontOnFocus 标志
    return (currentMode == mode) ? 0 : ImGuiWindowFlags_NoBringToFrontOnFocus;
}

void NetworkViewer::LoadNetworkFromFile(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        std::string jsonContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        networkMap.loadNetworkData(jsonContent); // Load data into NetworkMap
    } catch (const std::exception& e) {
        std::cerr << "Error loading JSON file: " << e.what() << std::endl;
    }
}

void NetworkViewer::SaveNetworkToFile(const std::string& filePath) {
    try {
        // 在保存前更新所有节点的位置信息
        networkMap.updateNodePositions();

        std::ofstream file(filePath);
        if (file.is_open()) {
            file << networkMap.getNetworkData();
            file.close();
            lastFilePath = filePath; // 更新最后使用的文件路径
            std::cout << "Network data saved successfully to: " << filePath << std::endl;
        } else {
            std::cerr << "Error opening file for writing: " << filePath << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error saving JSON file: " << e.what() << std::endl;
    }
}
