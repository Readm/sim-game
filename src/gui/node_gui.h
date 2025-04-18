#pragma once

#include <imgui.h>
#include <imgui_node_editor.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "../sim/include/node.h"

namespace ed = ax::NodeEditor;

class NodeGUI {
public:
    NodeGUI();
    ~NodeGUI();

    void init();
    void shutdown();
    void render(const std::shared_ptr<sim::Node>& node);

    // 节点绘制相关函数
    void drawNode(const std::shared_ptr<sim::Node>& node);
    void drawPorts(const std::shared_ptr<sim::Node>& node);
    void drawNodeProperties(const std::shared_ptr<sim::Node>& node);
    void drawChildNodes(const std::shared_ptr<sim::Node>& node);
    
    // 设置与获取
    void setNodePosition(sim::NodeID nodeId, const ImVec2& position);
    ImVec2 getNodePosition(sim::NodeID nodeId) const;
    
    // 事件处理
    void handleNodeCreated(const std::shared_ptr<sim::Node>& node);
    void handleNodeDeleted(sim::NodeID nodeId);
    void handleSelectionChanged();
    
    // 节点编辑功能
    void createConnection(sim::NodeID fromNodeId, const std::string& fromPortName,
                         sim::NodeID toNodeId, const std::string& toPortName);
    void deleteConnection(int connectionId);
    
    // 获取选中的节点
    std::shared_ptr<sim::Node> getSelectedNode() const;
    
    // 回调设置
    using NodeSelectedCallback = std::function<void(const std::shared_ptr<sim::Node>&)>;
    void setNodeSelectedCallback(NodeSelectedCallback callback);
    
private:
    // 内部数据结构
    struct NodeVisual {
        sim::NodeID id;
        std::string type;
        std::string name;
        ImVec2 position;
        ed::NodeId editorId;
        std::unordered_map<std::string, ed::PinId> inputPinIds;
        std::unordered_map<std::string, ed::PinId> outputPinIds;
    };
    
    struct ConnectionVisual {
        int id;
        sim::NodeID fromNodeId;
        std::string fromPortName;
        sim::NodeID toNodeId;
        std::string toPortName;
        ed::LinkId editorId;
    };
    
    // 编辑器上下文
    ed::EditorContext* editorContext_;
    
    // 可视化数据
    std::unordered_map<sim::NodeID, NodeVisual> nodeVisuals_;
    std::unordered_map<int, ConnectionVisual> connectionVisuals_;
    int nextConnectionId_;
    
    // 选中的节点
    sim::NodeID selectedNodeId_;
    
    // 回调函数
    NodeSelectedCallback nodeSelectedCallback_;
    
    // 样式设置
    float nodeWidth_ = 180.0f;
    float headerHeight_ = 24.0f;
    float portSize_ = 8.0f;
    
    // 辅助函数
    ImColor getNodeColor(const std::string& nodeType) const;
    ImColor getPortColor(sim::TypeID portTypeId) const;
    ed::PinId getPinId(sim::NodeID nodeId, const std::string& portName, bool isInput) const;
    NodeVisual& getOrCreateNodeVisual(const std::shared_ptr<sim::Node>& node);
    void updateNodeVisualLayout(NodeVisual& visual);
}; 