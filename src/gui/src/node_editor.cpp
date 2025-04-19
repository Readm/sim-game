#include "../include/node_editor.h"
#include "../include/node.h"
#include "../include/pin.h"
#include "../include/link.h"
#include <imgui.h>
#include <imgui_node_editor.h>
#include <algorithm>
#include <iostream>

namespace ed = ax::NodeEditor;

namespace sim {
namespace gui {

ed::EditorContext* NodeEditor::s_SharedContext = nullptr;

NodeEditor::NodeEditor() : m_Context(nullptr) {
    std::cout << "DEBUG: Creating NodeEditor object" << std::endl;
}

NodeEditor::~NodeEditor() {
    std::cout << "DEBUG: Destroying NodeEditor object" << std::endl;
    // 不要销毁共享上下文
    m_Context = nullptr;
}

bool NodeEditor::Initialize() {
    std::cout << "DEBUG: Initializing NodeEditor" << std::endl;
    
    if (m_Context) {
        return true;
    }

    if (!s_SharedContext) {
        ed::Config config;
        config.SettingsFile = "test_editor.json";
        s_SharedContext = ed::CreateEditor(&config);
        
        if (!s_SharedContext) {
            std::cerr << "Error: Failed to create shared editor context" << std::endl;
            return false;
        }
        
        std::cout << "DEBUG: Created shared editor context" << std::endl;
    }

    m_Context = s_SharedContext;
    std::cout << "DEBUG: NodeEditor initialization completed" << std::endl;
    return true;
}

void NodeEditor::Render() {
    if (!m_Context) {
        return;
    }

    // 确保ImGui窗口已经创建
    if (!ImGui::GetCurrentContext()) {
        std::cerr << "Error: ImGui context not initialized" << std::endl;
        return;
    }

    // 创建新的ImGui窗口
    ImGui::Begin("Node Editor Window");
    ed::Begin("Node Editor");
    
    // 渲染节点
    for (const auto& node : m_Nodes) {
        node->Render();
    }
    
    // 渲染连接
    for (const auto& link : m_Links) {
        link->Render();
    }
    
    ed::End();
    ImGui::End();
}

std::shared_ptr<Node> NodeEditor::CreateNode(const std::string& name, const ImVec2& position) {
    auto node = std::make_shared<Node>(name, Node::Type::Blueprint, position);
    m_Nodes.push_back(node);
    return node;
}

void NodeEditor::DeleteNode(std::shared_ptr<Node> node) {
    auto it = std::find(m_Nodes.begin(), m_Nodes.end(), node);
    if (it != m_Nodes.end()) {
        m_Nodes.erase(it);
    }
}

std::shared_ptr<Link> NodeEditor::CreateLink(std::shared_ptr<Pin> startPin, std::shared_ptr<Pin> endPin) {
    auto link = std::make_shared<Link>(startPin, endPin);
    m_Links.push_back(link);
    return link;
}

void NodeEditor::DeleteLink(std::shared_ptr<Link> link) {
    auto it = std::find(m_Links.begin(), m_Links.end(), link);
    if (it != m_Links.end()) {
        m_Links.erase(it);
    }
}

void NodeEditor::DrawNodes() {
    for (auto& node : m_Nodes) {
        ed::BeginNode(node->GetId());
        ImGui::Text("%s", node->GetName().c_str());
        ed::EndNode();
    }
}

void NodeEditor::DrawLinks() {
    for (auto& link : m_Links) {
        ed::Link(link->GetId(), link->GetStartPin()->GetId(), link->GetEndPin()->GetId());
    }
}

void NodeEditor::HandleNodeCreation() {
    // TODO: 实现节点创建逻辑
}

void NodeEditor::HandleLinkCreation() {
    // TODO: 实现连接创建逻辑
}

void NodeEditor::HandleDeletion() {
    // TODO: 实现删除逻辑
}

} // namespace gui
} // namespace sim 