#pragma once

#include <memory>
#include <string>
#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace sim {
namespace gui {

class Node;

class Pin {
public:
    enum class Type {
        Flow,
        Bool,
        Int,
        Float,
        String
    };

    Pin(const std::string& name, Type type, ed::PinKind kind, std::shared_ptr<Node> node);
    ~Pin();

    // 获取引脚ID
    ed::PinId GetId() const { return m_Id; }
    
    // 获取引脚名称
    const std::string& GetName() const { return m_Name; }
    
    // 获取引脚类型
    Type GetType() const { return m_Type; }
    
    // 获取引脚种类（输入/输出）
    ed::PinKind GetKind() const { return m_Kind; }
    
    // 获取所属节点
    std::shared_ptr<Node> GetNode() const { return m_Node; }

private:
    ed::PinId m_Id;
    std::string m_Name;
    Type m_Type;
    ed::PinKind m_Kind;
    std::shared_ptr<Node> m_Node;
};

} // namespace gui
} // namespace sim 