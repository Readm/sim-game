#ifndef NODE_UI_H
#define NODE_UI_H

#include "imgui.h"
#include <nlohmann/json.hpp>
#include <string>

class NodeUI {
public:
    NodeUI(const nlohmann::json& node, const ImVec2& windowPos);

    void render() const;

private:
    ImVec2 position;
    ImVec2 size;
    ImU32 color;
};

#endif // NODE_UI_H
