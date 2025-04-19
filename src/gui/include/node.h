#pragma once

#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace sim {
namespace gui {

class Pin;
class Link;

class Node : public std::enable_shared_from_this<Node> {
public:
    enum class Type {
        Blueprint,
        Simple,
        Tree,
        Comment,
        Houdini
    };

    Node(const std::string& name, Type type, const ImVec2& position);
    ~Node();

    // 获取节点ID
    ed::NodeId GetId() const { return m_Id; }
    
    // 获取节点名称
    const std::string& GetName() const { return m_Name; }
    
    // 获取节点类型
    Type GetType() const { return m_Type; }
    
    // 获取节点位置
    const ImVec2& GetPosition() const { return m_Position; }
    
    // 设置节点位置
    void SetPosition(const ImVec2& position) { m_Position = position; }
    
    // 添加输入引脚
    void AddInputPin(const std::string& name, ed::PinKind kind);
    
    // 添加输出引脚
    void AddOutputPin(const std::string& name, ed::PinKind kind);
    
    // 获取输入引脚
    const std::vector<std::shared_ptr<Pin>>& GetInputPins() const { return m_InputPins; }
    
    // 获取输出引脚
    const std::vector<std::shared_ptr<Pin>>& GetOutputPins() const { return m_OutputPins; }

    // 渲染节点
    void Render();

private:
    ed::NodeId m_Id;
    std::string m_Name;
    Type m_Type;
    ImVec2 m_Position;
    std::vector<std::shared_ptr<Pin>> m_InputPins;
    std::vector<std::shared_ptr<Pin>> m_OutputPins;
};

// 添加operator<<重载
inline std::ostream& operator<<(std::ostream& os, const Node& node) {
    return os << "Node(" << node.GetName() << ")";
}

} // namespace gui
} // namespace sim 