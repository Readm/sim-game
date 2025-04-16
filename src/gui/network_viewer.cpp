#include "network_viewer.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

NetworkViewer::NetworkViewer()
    : m_Context(nullptr)
    , m_IsSimulationRunning(false)
    , m_CurrentTick(0)
{
}

NetworkViewer::~NetworkViewer()
{
    shutdown();
}

void NetworkViewer::init()
{
    // 创建节点编辑器上下文
    ed::Config config;
    config.SettingsFile = "network_editor.json";
    m_Context = ed::CreateEditor(&config);
}

void NetworkViewer::shutdown()
{
    // 断开与服务器的连接
    disconnectFromServer();
    
    // 销毁节点编辑器上下文
    if (m_Context)
    {
        ed::DestroyEditor(m_Context);
        m_Context = nullptr;
    }
}

void NetworkViewer::render()
{
    // 渲染菜单栏
    renderMenuBar();
    
    // 渲染主界面
    ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("网络仿真查看器", nullptr, ImGuiWindowFlags_MenuBar))
    {
        // 显示连接状态
        renderConnectionStatus();
        
        // 渲染模拟控制面板
        renderSimulationControl();
        
        // 渲染节点编辑器
        renderNodeEditor();
        
        // 渲染属性面板
        renderPropertyPanel();
    }
    ImGui::End();
}

// 网络连接相关方法
bool NetworkViewer::connectToServer(const std::string& url) {
    // 创建网络客户端
    if (!m_NetworkClient) {
        m_NetworkClient = std::make_unique<NetworkClient>(url);
    }
    
    // 尝试连接
    bool success = m_NetworkClient->connect();
    
    if (success) {
        std::cout << "已成功连接到服务器: " << url << std::endl;
        
        // 设置状态更新回调
        m_NetworkClient->setStateUpdateCallback([this](const json& state) {
            this->handleStateUpdate(state);
        });
        
        // 获取初始网络状态
        json initialState = m_NetworkClient->getNetworkState();
        
        // 解析状态
        if (initialState != json("{}")) {
            updateNetworkState(initialState.dump());
        }
    } else {
        std::cerr << "连接到服务器失败: " << url << std::endl;
    }
    
    return success;
}

void NetworkViewer::disconnectFromServer() {
    if (m_NetworkClient) {
        m_NetworkClient->disconnect();
        m_NetworkClient.reset();
        std::cout << "已断开与服务器的连接" << std::endl;
    }
}

void NetworkViewer::startSimulation()
{
    if (m_NetworkClient && m_NetworkClient->isConnected()) {
        bool success = m_NetworkClient->startSimulation();
        if (success) {
            m_IsSimulationRunning = true;
        }
    } else {
        // 本地模式下直接设置状态
        m_IsSimulationRunning = true;
    }
}

void NetworkViewer::stopSimulation()
{
    if (m_NetworkClient && m_NetworkClient->isConnected()) {
        bool success = m_NetworkClient->stopSimulation();
        if (success) {
            m_IsSimulationRunning = false;
        }
    } else {
        // 本地模式下直接设置状态
        m_IsSimulationRunning = false;
    }
}

void NetworkViewer::stepSimulation()
{
    if (m_NetworkClient && m_NetworkClient->isConnected()) {
        m_NetworkClient->stepSimulation();
        m_CurrentTick++;
    } else {
        // 本地模式下直接更新tick
        m_CurrentTick++;
    }
}

void NetworkViewer::resetSimulation()
{
    if (m_NetworkClient && m_NetworkClient->isConnected()) {
        m_NetworkClient->resetSimulation();
        m_CurrentTick = 0;
        m_IsSimulationRunning = false;
    } else {
        // 本地模式下直接重置状态
        m_CurrentTick = 0;
        m_IsSimulationRunning = false;
    }
}

void NetworkViewer::updateNetworkState(const std::string& stateStr)
{
    try {
        std::cout << "收到状态更新: " << stateStr << std::endl;
        json state(stateStr);
        
        // 解析状态
        if (stateStr.find("running") != std::string::npos) {
            m_IsSimulationRunning = stateStr.find("\"running\":true") != std::string::npos;
        }
        
        // 设置当前tick，如果存在的话
        if (stateStr.find("tick") != std::string::npos) {
            size_t pos = stateStr.find("\"tick\":");
            if (pos != std::string::npos) {
                pos += 7; // 跳过 "tick":
                size_t endPos = stateStr.find(",", pos);
                if (endPos == std::string::npos) {
                    endPos = stateStr.find("}", pos);
                }
                if (endPos != std::string::npos) {
                    std::string tickStr = stateStr.substr(pos, endPos - pos);
                    try {
                        m_CurrentTick = std::stoi(tickStr);
                        std::cout << "Tick更新为: " << m_CurrentTick << std::endl;
                    } catch (...) {
                        std::cerr << "无法解析tick值: " << tickStr << std::endl;
                    }
                }
            }
        }
        
        // 这里还会解析节点和连接信息...
        
    } catch (const std::exception& e) {
        std::cerr << "解析网络状态失败: " << e.what() << std::endl;
    }
}

void NetworkViewer::handleStateUpdate(const json& state) {
    // 解析更新的状态
    std::cout << "收到状态更新: " << state.dump() << std::endl;
    
    // 在这里我们会解析状态并更新UI...
}

void NetworkViewer::renderMenuBar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("文件"))
        {
            if (ImGui::MenuItem("新建", "Ctrl+N")) {}
            if (ImGui::MenuItem("打开", "Ctrl+O")) {}
            if (ImGui::MenuItem("保存", "Ctrl+S")) {}
            if (ImGui::MenuItem("另存为...", "Ctrl+Shift+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("退出", "Alt+F4")) {}
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("编辑"))
        {
            if (ImGui::MenuItem("撤销", "Ctrl+Z")) {}
            if (ImGui::MenuItem("重做", "Ctrl+Y")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("剪切", "Ctrl+X")) {}
            if (ImGui::MenuItem("复制", "Ctrl+C")) {}
            if (ImGui::MenuItem("粘贴", "Ctrl+V")) {}
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("模拟"))
        {
            if (ImGui::MenuItem("启动", "F5", &m_IsSimulationRunning)) { if(m_IsSimulationRunning) startSimulation(); else stopSimulation(); }
            if (ImGui::MenuItem("停止", "F6")) { stopSimulation(); }
            if (ImGui::MenuItem("单步", "F7")) { stepSimulation(); }
            if (ImGui::MenuItem("重置", "F8")) { resetSimulation(); }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("帮助"))
        {
            if (ImGui::MenuItem("关于")) {}
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }
}

void NetworkViewer::renderConnectionStatus() {
    ImGui::BeginChild("连接状态", ImVec2(ImGui::GetContentRegionAvail().x, 40), true);
    
    ImGui::Text("服务器连接: %s", isConnected() ? "已连接" : "未连接");
    
    if (isConnected()) {
        ImGui::SameLine(200);
        if (ImGui::Button("断开连接")) {
            disconnectFromServer();
        }
    } else {
        static char serverUrl[128] = "http://localhost:8080";
        ImGui::SameLine(200);
        ImGui::PushItemWidth(300);
        ImGui::InputText("##url", serverUrl, sizeof(serverUrl));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("连接")) {
            connectToServer(serverUrl);
        }
    }
    
    ImGui::EndChild();
}

void NetworkViewer::renderSimulationControl()
{
    ImGui::BeginChild("模拟控制", ImVec2(ImGui::GetContentRegionAvail().x, 60), true);
    
    // 显示当前状态
    ImGui::Text("模拟状态: %s", m_IsSimulationRunning ? "运行中" : "已停止");
    ImGui::SameLine(200);
    ImGui::Text("当前Tick: %d", m_CurrentTick);
    
    // 控制按钮
    if (!m_IsSimulationRunning)
    {
        if (ImGui::Button("启动", ImVec2(60, 0)))
        {
            startSimulation();
        }
    }
    else
    {
        if (ImGui::Button("停止", ImVec2(60, 0)))
        {
            stopSimulation();
        }
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("单步", ImVec2(60, 0)))
    {
        stepSimulation();
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("重置", ImVec2(60, 0)))
    {
        resetSimulation();
    }
    
    ImGui::EndChild();
}

void NetworkViewer::renderNodeEditor()
{
    ImGui::Text("节点编辑器");
    
    ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 200);
    ImGui::BeginChild("节点编辑器区域", size, true);
    
    ed::SetCurrentEditor(m_Context);
    ed::Begin("网络编辑器");
    
    // 这里渲染节点和连接
    // ...
    
    ed::End();
    ed::SetCurrentEditor(nullptr);
    
    ImGui::EndChild();
}

void NetworkViewer::renderPropertyPanel()
{
    ImGui::Text("属性面板");
    
    ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 150);
    ImGui::BeginChild("属性面板区域", size, true);
    
    // 显示选中节点的属性
    // ...
    
    ImGui::EndChild();
} 