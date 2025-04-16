#pragma once

#include <imgui.h>
#include <imgui_node_editor.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "network_client.h"

namespace ed = ax::NodeEditor;

class NetworkViewer {
public:
    NetworkViewer();
    ~NetworkViewer();

    void init();
    void shutdown();
    void render();

    // 网络控制
    void startSimulation();
    void stopSimulation();
    void stepSimulation();
    void resetSimulation();

    // 连接管理
    bool connectToServer(const std::string& url = "http://localhost:8080");
    void disconnectFromServer();
    
    // 状态更新
    void updateNetworkState(const std::string& state);
    
    // 获取状态 (用于测试)
    bool isSimulationRunning() const { return m_IsSimulationRunning; }
    int getCurrentTick() const { return m_CurrentTick; }
    bool isConnected() const { return m_NetworkClient ? m_NetworkClient->isConnected() : false; }

private:
    // GUI组件
    void renderMenuBar();
    void renderSimulationControl();
    void renderNodeEditor();
    void renderPropertyPanel();
    void renderConnectionStatus();

    // 节点编辑器上下文
    ed::EditorContext* m_Context;

    // 网络客户端
    std::unique_ptr<NetworkClient> m_NetworkClient;
    
    // 模拟状态
    bool m_IsSimulationRunning;
    int m_CurrentTick;
    bool m_IsConnected = false;
    
    // 状态更新回调
    void handleStateUpdate(const json& state);

    // 节点数据
    struct NodeInfo {
        int id;
        std::string name;
        std::string type;
        ImVec2 position;
        std::vector<int> inputs;
        std::vector<int> outputs;
    };

    struct LinkInfo {
        int id;
        int startPin;
        int endPin;
    };

    std::unordered_map<int, NodeInfo> m_Nodes;
    std::unordered_map<int, LinkInfo> m_Links;
}; 