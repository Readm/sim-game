#include "node_renderer.h"
#include "blueprints-example.h"
#include <imgui_internal.h>

// NodeRenderer 基类实现
void NodeRenderer::RenderHeader(Node* node) {
    ImGui::Text("%s", node->Name.c_str());
}

void NodeRenderer::RenderInputs(Node* node) {
    for (auto& input : node->Inputs) {
        ImGui::Text("%s", input.Name.c_str());
    }
}

void NodeRenderer::RenderOutputs(Node* node) {
    for (auto& output : node->Outputs) {
        ImGui::Text("%s", output.Name.c_str());
    }
}

void NodeRenderer::RenderMiddle(Node* node) {
    // 基类中的默认实现
    ImGui::Text("Node Content");
}

// BlueprintNodeRenderer 实现
void BlueprintNodeRenderer::RenderNode(Node* node, const RenderContext& context) {
    // 设置节点位置
    ImGui::SetCursorPos(context.Position);
    
    // 开始节点渲染
    ed::BeginNode(node->ID);
    
    // 渲染节点头部
    RenderBlueprintHeader(node);
    
    // 渲染输入端口
    RenderBlueprintInputs(node);
    
    // 渲染中间内容区域
    RenderContent(node, context);
    
    // 如果节点展开，渲染子节点
    if (node->IsExpanded) {
        RenderChildren(node, context);
    }
    
    // 渲染输出端口
    RenderBlueprintOutputs(node);
    
    // 结束节点渲染
    ed::EndNode();
}

void BlueprintNodeRenderer::RenderChildren(Node* node, const RenderContext& context) {
    // 在节点的Middle区域中渲染子节点
    ImGui::BeginChild("Children", ImVec2(0, 0), true);
    
    RenderContext childContext = context;
    childContext.Depth++;
    childContext.IsChildNode = true;
    childContext.ParentSize = ImGui::GetContentRegionAvail();
    
    for (auto& child : node->Children) {
        RenderNode(child, childContext);
    }
    
    ImGui::EndChild();
}

void BlueprintNodeRenderer::RenderContent(Node* node, const RenderContext& context) {
    RenderBlueprintMiddle(node);
}

void BlueprintNodeRenderer::RenderConnections(Node* node, const RenderContext& context) {
    // 渲染节点之间的连接
    for (auto& input : node->Inputs) {
        if (input.Link) {
            ed::Link(input.Link->ID, input.Link->InputPin->ID, input.Link->OutputPin->ID);
        }
    }
}

void BlueprintNodeRenderer::RenderBlueprintHeader(Node* node) {
    ImGui::PushStyleColor(ImGuiCol_Header, node->Color);
    ImGui::Text("%s", node->Name.c_str());
    ImGui::PopStyleColor();
}

void BlueprintNodeRenderer::RenderBlueprintInputs(Node* node) {
    for (auto& input : node->Inputs) {
        ImGui::BeginGroup();
        ImGui::Text("%s", input.Name.c_str());
        ed::BeginPin(input.ID, ed::PinKind::Input);
        ImGui::EndGroup();
        ed::EndPin();
    }
}

void BlueprintNodeRenderer::RenderBlueprintOutputs(Node* node) {
    for (auto& output : node->Outputs) {
        ImGui::BeginGroup();
        ImGui::Text("%s", output.Name.c_str());
        ed::BeginPin(output.ID, ed::PinKind::Output);
        ImGui::EndGroup();
        ed::EndPin();
    }
}

void BlueprintNodeRenderer::RenderBlueprintMiddle(Node* node) {
    // 渲染节点的中间内容
    if (node->nodeData.contains("content")) {
        ImGui::Text("%s", node->nodeData["content"].get<std::string>().c_str());
    }
} 