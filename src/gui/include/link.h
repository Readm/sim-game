#pragma once

#include <memory>
#include <iostream>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace sim {
namespace gui {

class Pin;

class Link {
public:
    Link(const std::shared_ptr<Pin>& startPin, const std::shared_ptr<Pin>& endPin);
    ~Link();

    // 获取连接ID
    ed::LinkId GetId() const { return m_Id; }
    
    // 获取起始引脚
    const std::shared_ptr<Pin>& GetStartPin() const { return m_StartPin; }
    
    // 获取结束引脚
    const std::shared_ptr<Pin>& GetEndPin() const { return m_EndPin; }

    // 渲染连接
    void Render();

private:
    ed::LinkId m_Id;
    std::shared_ptr<Pin> m_StartPin;
    std::shared_ptr<Pin> m_EndPin;
};

// 添加operator<<重载
inline std::ostream& operator<<(std::ostream& os, const Link& link) {
    return os << "Link(" << link.GetId().AsPointer() << ")";
}

} // namespace gui
} // namespace sim 