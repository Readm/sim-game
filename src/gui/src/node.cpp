#include "../include/node.h"
#include "../include/pin.h"
#include <imgui.h>
#include <imgui_node_editor.h>
#include <iostream>

namespace ed = ax::NodeEditor;

namespace sim {
namespace gui {

static int nextNodeId = 1;

Node::Node(const std::string& name, Type type, const ImVec2& position)
    : m_Id(nextNodeId++), m_Name(name), m_Type(type), m_Position(position) {
    std::cout << "DEBUG: Creating Node: " << name << std::endl;
}

Node::~Node() {
    std::cout << "DEBUG: Destroying Node: " << m_Name << std::endl;
    // 清理引脚
    m_InputPins.clear();
    m_OutputPins.clear();
}

void Node::AddInputPin(const std::string& name, ed::PinKind kind) {
    m_InputPins.push_back(std::make_shared<Pin>(name, Pin::Type::Flow, kind, shared_from_this()));
}

void Node::AddOutputPin(const std::string& name, ed::PinKind kind) {
    m_OutputPins.push_back(std::make_shared<Pin>(name, Pin::Type::Flow, kind, shared_from_this()));
}

void Node::Render() {
    ed::BeginNode(m_Id);
    
    // 绘制节点标题
    ImGui::TextUnformatted(m_Name.c_str());
    
    // 绘制输入引脚
    for (const auto& pin : m_InputPins) {
        ed::BeginPin(pin->GetId(), ed::PinKind::Input);
        ImGui::TextUnformatted(pin->GetName().c_str());
        ed::EndPin();
    }
    
    // 绘制输出引脚
    for (const auto& pin : m_OutputPins) {
        ed::BeginPin(pin->GetId(), ed::PinKind::Output);
        ImGui::TextUnformatted(pin->GetName().c_str());
        ed::EndPin();
    }
    
    ed::EndNode();
}

} // namespace gui
} // namespace sim 