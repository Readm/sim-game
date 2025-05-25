#pragma once

#include <imgui.h>
#include <imgui_node_editor.h>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace ed = ax::NodeEditor;
using json = nlohmann::json;

// 节点类型
enum class NodeType {
    Blueprint,
    Simple,
    Comment,
    Houdini
};

// 前向声明
struct Link;

// 端口结构
struct Pin {
    ed::PinId ID;
    std::string Name;
    Link* Link{nullptr};
};

// 连接结构
struct Link {
    ed::LinkId ID;
    Pin* InputPin{nullptr};
    Pin* OutputPin{nullptr};
};

// 节点结构
struct Node {
    ed::NodeId ID;
    std::string Name;
    std::vector<Pin> Inputs;
    std::vector<Pin> Outputs;
    ImColor Color;
    NodeType Type;
    ImVec2 Size;
    
    // 父子关系
    Node* Parent{nullptr};
    std::vector<Node*> Children;
    
    // 节点状态
    bool IsExpanded{true};
    json nodeData;
    
    // 渲染状态
    struct {
        ImVec2 Position;
        ImVec2 Size;
        bool IsVisible;
        float Opacity;
    } RenderState;
};

// 函数声明
void Init();
void Shutdown();
void OnFrame();
void LoadNodesFromJson();
void SaveNodesToJson();
void CreateLink(ed::PinId inputPinId, ed::PinId outputPinId);
void DeleteLink(ed::LinkId linkId);
void DeleteNode(ed::NodeId nodeId);
int GetNextId(); 