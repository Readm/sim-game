#pragma once

#include <imgui.h>
#include <imgui_node_editor.h>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace ed = ax::NodeEditor;

// 前向声明
struct Node;
struct Pin;

// 渲染上下文
struct RenderContext {
    ImVec2 Position;      // 渲染位置
    float Depth;          // 渲染深度
    bool IsChildNode;     // 是否为子节点
    ImVec2 ParentSize;    // 父节点大小
    ImVec2 AvailableSpace;// 可用空间
};

// 渲染器基类
class NodeRenderer {
public:
    virtual ~NodeRenderer() = default;
    
    // 渲染节点的主要接口
    virtual void RenderNode(Node* node, const RenderContext& context) = 0;
    
    // 渲染子节点
    virtual void RenderChildren(Node* node, const RenderContext& context) = 0;
    
    // 渲染节点内容
    virtual void RenderContent(Node* node, const RenderContext& context) = 0;
    
    // 渲染节点连接
    virtual void RenderConnections(Node* node, const RenderContext& context) = 0;
    
protected:
    // 辅助渲染函数
    void RenderHeader(Node* node);
    void RenderInputs(Node* node);
    void RenderOutputs(Node* node);
    void RenderMiddle(Node* node);
};

// 蓝图节点渲染器
class BlueprintNodeRenderer : public NodeRenderer {
public:
    BlueprintNodeRenderer() = default;
    ~BlueprintNodeRenderer() override = default;
    
    void RenderNode(Node* node, const RenderContext& context) override;
    void RenderChildren(Node* node, const RenderContext& context) override;
    void RenderContent(Node* node, const RenderContext& context) override;
    void RenderConnections(Node* node, const RenderContext& context) override;
    
private:
    // 蓝图特定的渲染辅助函数
    void RenderBlueprintHeader(Node* node);
    void RenderBlueprintInputs(Node* node);
    void RenderBlueprintOutputs(Node* node);
    void RenderBlueprintMiddle(Node* node);
}; 