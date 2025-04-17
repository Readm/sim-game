#include "network_viewer.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include "http_client.h"
#include <nlohmann/json.hpp>

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

bool NetworkViewer::init()
{
    // 创建节点编辑器上下文
    ed::Config config;
    config.SettingsFile = "network_editor.json";
    m_Context = ed::CreateEditor(&config);
    return m_Context != nullptr;
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
bool NetworkViewer::connectToServer() {
    // 使用默认URL
    return connectToServer("http://localhost:8080");
}

bool NetworkViewer::connectToServer(const std::string& url) {
    try {
        // 创建网络客户端
        if (!m_NetworkClient) {
            std::cout << "创建新的网络客户端连接到: " << url << std::endl;
            m_NetworkClient = std::make_unique<NetworkClient>(url);
        }
        
        // 尝试连接
        std::cout << "尝试连接到服务器: " << url << std::endl;
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
                std::cout << "收到初始网络状态: " << initialState.dump() << std::endl;
                updateNetworkState(initialState.dump());
            } else {
                std::cout << "初始网络状态为空" << std::endl;
            }
        } else {
            std::cerr << "连接到服务器失败: " << url << std::endl;
        }
        
        return success;
    } catch (const std::exception& e) {
        std::cerr << "连接服务器时发生异常: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "连接服务器时发生未知异常" << std::endl;
        return false;
    }
}

void NetworkViewer::disconnectFromServer() {
    if (m_NetworkClient) {
        m_NetworkClient->disconnect();
        m_NetworkClient.reset();
        std::cout << "已断开与服务器的连接" << std::endl;
    }
}

bool NetworkViewer::startSimulation()
{
    if (m_NetworkClient && m_NetworkClient->isConnected()) {
        bool success = m_NetworkClient->startSimulation();
        if (success) {
            m_IsSimulationRunning = true;
        }
        return success;
    } else {
        // 本地模式下直接设置状态
        m_IsSimulationRunning = true;
        return true;
    }
}

bool NetworkViewer::stopSimulation()
{
    if (m_NetworkClient && m_NetworkClient->isConnected()) {
        bool success = m_NetworkClient->stopSimulation();
        if (success) {
            m_IsSimulationRunning = false;
        }
        return success;
    } else {
        // 本地模式下直接设置状态
        m_IsSimulationRunning = false;
        return true;
    }
}

bool NetworkViewer::stepSimulation()
{
    if (m_NetworkClient && m_NetworkClient->isConnected()) {
        bool success = m_NetworkClient->stepSimulation();
        if (success) {
            m_CurrentTick++;
        }
        return success;
    } else {
        // 本地模式下直接更新tick
        m_CurrentTick++;
        return true;
    }
}

bool NetworkViewer::resetSimulation()
{
    if (m_NetworkClient && m_NetworkClient->isConnected()) {
        bool success = m_NetworkClient->resetSimulation();
        if (success) {
            m_CurrentTick = 0;
            m_IsSimulationRunning = false;
        }
        return success;
    } else {
        // 本地模式下直接重置状态
        m_CurrentTick = 0;
        m_IsSimulationRunning = false;
        return true;
    }
}

void NetworkViewer::updateNetworkState(const std::string& stateStr)
{
    try {
        std::cout << "收到状态更新: " << stateStr << std::endl;
        
        // 解析模拟状态
        if (stateStr.find("running") != std::string::npos) {
            m_IsSimulationRunning = stateStr.find("\"running\":true") != std::string::npos;
        }
        
        // 解析当前tick
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
        
        // 解析节点信息
        clearNetworkState();
        
        // 使用nlohmann::json仅用于解析，不暴露给其他代码
        auto tempJson = nlohmann::json::parse(stateStr);
        
        // 在data字段中找到节点信息
        nlohmann::json nodesJson;
        if (tempJson.contains("data") && tempJson["data"].contains("nodes")) {
            nodesJson = tempJson["data"]["nodes"];
        } else if (tempJson.contains("nodes")) {
            nodesJson = tempJson["nodes"];
        }
        
        if (!nodesJson.empty() && nodesJson.is_array()) {
            for (const auto& nodeJson : nodesJson) {
                Node node;
                node.id = nodeJson["id"].get<int>();
                node.name = nodeJson.value("name", "未命名节点");
                node.type = nodeJson.value("type", "未知类型");
                node.editorId = ed::NodeId(node.id);
                
                // 解析节点位置
                if (nodeJson.contains("position") && nodeJson["position"].is_object()) {
                    float x = nodeJson["position"].value("x", 0.0f);
                    float y = nodeJson["position"].value("y", 0.0f);
                    node.position = ImVec2(x, y);
                } else {
                    // 随机位置
                    node.position = ImVec2(100.0f + (rand() % 500), 100.0f + (rand() % 300));
                }
                
                // 解析节点属性
                if (nodeJson.contains("properties") && nodeJson["properties"].is_object()) {
                    node.properties = nodeJson["properties"];
                }
                
                // 解析端口
                if (nodeJson.contains("inputs") && nodeJson["inputs"].is_array()) {
                    for (const auto& portJson : nodeJson["inputs"]) {
                        Port port;
                        port.id = portJson["id"].get<int>();
                        port.name = portJson.value("name", "输入");
                        port.type = portJson.value("type", "any");
                        port.isInput = true;
                        port.nodeId = node.id;
                        port.connected = false;
                        port.editorId = ed::PinId(port.id);
                        
                        ports_[port.id] = port;
                        node.inputPorts.push_back(port.id);
                    }
                }
                
                if (nodeJson.contains("outputs") && nodeJson["outputs"].is_array()) {
                    for (const auto& portJson : nodeJson["outputs"]) {
                        Port port;
                        port.id = portJson["id"].get<int>();
                        port.name = portJson.value("name", "输出");
                        port.type = portJson.value("type", "any");
                        port.isInput = false;
                        port.nodeId = node.id;
                        port.connected = false;
                        port.editorId = ed::PinId(port.id);
                        
                        ports_[port.id] = port;
                        node.outputPorts.push_back(port.id);
                    }
                }
                
                // 添加节点到映射
                nodes_[node.id] = node;
            }
        }
        
        // 解析连接
        nlohmann::json connectionsJson;
        if (tempJson.contains("data") && tempJson["data"].contains("connections")) {
            connectionsJson = tempJson["data"]["connections"];
        } else if (tempJson.contains("connections")) {
            connectionsJson = tempJson["connections"];
        }
        
        if (!connectionsJson.empty() && connectionsJson.is_array()) {
            for (const auto& connJson : connectionsJson) {
                Connection conn;
                conn.id = connJson["id"].get<int>();
                
                // 检查不同可能的字段名
                if (connJson.contains("sourceNodeId") && connJson.contains("sourcePortId")) {
                    conn.fromNodeId = connJson["sourceNodeId"].get<int>();
                    conn.fromPortId = connJson["sourcePortId"].get<int>();
                } else if (connJson.contains("from_node") && connJson.contains("from_port")) {
                    conn.fromNodeId = connJson["from_node"].get<int>();
                    conn.fromPortId = connJson["from_port"].get<int>();
                }
                
                if (connJson.contains("targetNodeId") && connJson.contains("targetPortId")) {
                    conn.toNodeId = connJson["targetNodeId"].get<int>();
                    conn.toPortId = connJson["targetPortId"].get<int>();
                } else if (connJson.contains("to_node") && connJson.contains("to_port")) {
                    conn.toNodeId = connJson["to_node"].get<int>();
                    conn.toPortId = connJson["to_port"].get<int>();
                }
                
                conn.editorId = ed::LinkId(conn.id);
                
                // 标记端口为已连接
                if (ports_.find(conn.fromPortId) != ports_.end()) {
                    ports_[conn.fromPortId].connected = true;
                }
                
                if (ports_.find(conn.toPortId) != ports_.end()) {
                    ports_[conn.toPortId].connected = true;
                }
                
                // 添加连接到映射
                connections_[conn.id] = conn;
            }
        }
        
        // 调用状态更新回调
        if (stateUpdateCallback_) {
            // 将nlohmann::json转换为自定义json对象
            json customJson(tempJson.dump());
            stateUpdateCallback_(customJson);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "解析网络状态失败: " << e.what() << std::endl;
    }
}

void NetworkViewer::clearNetworkState() {
    nodes_.clear();
    ports_.clear();
    connections_.clear();
}

void NetworkViewer::handleStateUpdate(const json& state) {
    // 解析更新的状态
    std::cout << "收到状态更新: " << state.dump() << std::endl;
    
    // 将JSON对象转换为字符串，并调用updateNetworkState
    updateNetworkState(state.dump());
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
    
    // 渲染节点
    for (const auto& nodeEntry : nodes_)
    {
        const Node& node = nodeEntry.second;
        ed::BeginNode(node.editorId);
        
        // 设置节点位置
        ed::SetNodePosition(node.editorId, node.position);
        
        // 节点标题
        ImGui::BeginGroup();
        ImGui::TextUnformatted(node.name.c_str());
        ImGui::EndGroup();
        
        // 输入引脚
        ImGui::BeginGroup();
        for (int portId : node.inputPorts)
        {
            if (ports_.find(portId) != ports_.end())
            {
                const Port& port = ports_[portId];
                ed::BeginPin(port.editorId, ed::PinKind::Input);
                ImGui::BeginHorizontal(port.name.c_str());
                
                // 绘制引脚圆点
                ImGui::Spring(0);
                ImGui::Text("●");
                ImGui::Spring(0);
                
                // 显示引脚名称
                ImGui::Text("%s", port.name.c_str());
                ImGui::Spring(1);
                ImGui::EndHorizontal();
                ed::EndPin();
            }
        }
        ImGui::EndGroup();
        
        // 输出引脚
        ImGui::SameLine();
        ImGui::BeginGroup();
        for (int portId : node.outputPorts)
        {
            if (ports_.find(portId) != ports_.end())
            {
                const Port& port = ports_[portId];
                ed::BeginPin(port.editorId, ed::PinKind::Output);
                ImGui::BeginHorizontal(port.name.c_str());
                
                // 显示引脚名称
                ImGui::Spring(1);
                ImGui::Text("%s", port.name.c_str());
                ImGui::Spring(0);
                
                // 绘制引脚圆点
                ImGui::Text("●");
                ImGui::Spring(0);
                
                ImGui::EndHorizontal();
                ed::EndPin();
            }
        }
        ImGui::EndGroup();
        
        ed::EndNode();
    }
    
    // 渲染连接
    for (const auto& connEntry : connections_)
    {
        const Connection& conn = connEntry.second;
        
        // 确保两端的端口都存在
        if (ports_.find(conn.fromPortId) != ports_.end() && 
            ports_.find(conn.toPortId) != ports_.end())
        {
            const Port& fromPort = ports_[conn.fromPortId];
            const Port& toPort = ports_[conn.toPortId];
            
            ed::Link(conn.editorId, fromPort.editorId, toPort.editorId);
        }
    }
    
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

void NetworkViewer::setStateUpdateCallback(std::function<void(const json&)> callback) {
    stateUpdateCallback_ = callback;
} 