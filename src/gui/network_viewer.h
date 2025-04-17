#pragma once

#include <imgui.h>
#include <imgui_node_editor.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "network_client.h"
#include <functional>
#include <nlohmann/json.hpp>

namespace ed = ax::NodeEditor;

class HttpClient;

/**
 * @brief 网络可视化器类
 * 
 * 用于可视化网络状态和节点连接
 */
class NetworkViewer {
public:
    NetworkViewer();
    ~NetworkViewer();

    bool init();
    void shutdown();
    void render();

    // 网络控制
    bool startSimulation();
    bool stopSimulation();
    bool stepSimulation();
    bool resetSimulation();

    // 连接管理
    bool connectToServer(const std::string& url);
    bool connectToServer();  // 使用默认URL（为向后兼容保留）
    void disconnectFromServer();
    
    // 状态更新
    void updateNetworkState(const std::string& jsonStr);
    
    // 获取状态 (用于测试)
    bool isSimulationRunning() const { return m_IsSimulationRunning; }
    int getCurrentTick() const { return m_CurrentTick; }
    bool isConnected() const { return m_NetworkClient ? m_NetworkClient->isConnected() : false; }

    // 设置回调函数
    void setStateUpdateCallback(std::function<void(const json&)> callback);

    // 获取节点信息
    std::string getNodeInfo(int nodeId) const;

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

    // 内部数据结构
    struct Node {
        int id;
        std::string type;
        std::string name;
        nlohmann::json properties;
        ax::NodeEditor::NodeId editorId;
        ImVec2 position;
        std::vector<int> inputPorts;
        std::vector<int> outputPorts;
    };
    
    struct Port {
        int id;
        std::string name;
        std::string type;
        bool isInput;
        int nodeId;
        bool connected;
        ax::NodeEditor::PinId editorId;
    };
    
    struct Connection {
        int id;
        int fromNodeId;
        int fromPortId;
        int toNodeId;
        int toPortId;
        ax::NodeEditor::LinkId editorId;
    };
    
    // 内部方法
    void clearNetworkState();
    void parseNetworkState(const nlohmann::json& state);
    void createNodeVisual(Node& node);
    void drawNodeContent(const Node& node);
    ImColor getNodeColor(const std::string& nodeType) const;
    ImColor getPortColor(const std::string& portType) const;
    
    // 成员变量
    std::unique_ptr<HttpClient> client_;
    bool connected_;
    std::unordered_map<int, Node> nodes_;
    std::unordered_map<int, Port> ports_;
    std::unordered_map<int, Connection> connections_;
    ax::NodeEditor::EditorContext* editorContext_;
    std::function<void(const json&)> stateUpdateCallback_;
    
    // 全局样式设置
    float nodeWidth_ = 180.0f;
    float headerHeight_ = 24.0f;
    float portSize_ = 8.0f;
}; 