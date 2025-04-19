#include "../include/link.h"
#include "../include/pin.h"
#include <imgui_node_editor.h>
#include <iostream>

namespace ed = ax::NodeEditor;

namespace sim {
namespace gui {

static int nextLinkId = 1;

Link::Link(const std::shared_ptr<Pin>& startPin, const std::shared_ptr<Pin>& endPin)
    : m_Id(nextLinkId++), m_StartPin(startPin), m_EndPin(endPin) {
    std::cout << "DEBUG: Creating Link: " << m_Id.AsPointer() << std::endl;
}

Link::~Link() {
    std::cout << "DEBUG: Destroying Link: " << m_Id.AsPointer() << std::endl;
}

void Link::Render() {
    ed::Link(m_Id, m_StartPin->GetId(), m_EndPin->GetId());
}

} // namespace gui
} // namespace sim 