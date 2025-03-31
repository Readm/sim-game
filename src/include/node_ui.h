#ifndef NODE_UI_H
#define NODE_UI_H

#include "imgui.h"
#include <nlohmann/json.hpp>
#include <string>

class NodeUI {
public:
    static void renderNode(const nlohmann::json& node, const ImVec2& windowPos);
};

#endif // NODE_UI_H
