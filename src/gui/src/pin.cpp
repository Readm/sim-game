#include "../include/pin.h"
#include "../include/node.h"
#include <imgui.h>
#include <imgui_node_editor.h>
#include <iostream>

namespace ed = ax::NodeEditor;

namespace sim {
namespace gui {

static int nextPinId = 1;

Pin::Pin(const std::string& name, Type type, ed::PinKind kind, std::shared_ptr<Node> node)
    : m_Id(nextPinId++)
    , m_Name(name)
    , m_Type(type)
    , m_Kind(kind)
    , m_Node(node)
{
    std::cout << "DEBUG: Creating Pin: " << name << std::endl;
}

Pin::~Pin() {
    std::cout << "DEBUG: Destroying Pin: " << m_Name << std::endl;
}

} // namespace gui
} // namespace sim 