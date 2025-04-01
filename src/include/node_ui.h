#pragma once
#include <nlohmann/json.hpp>
#include <imgui.h>
#include <string>

class NodeUI {
public:
    NodeUI(const nlohmann::json& nodeData, const ImVec2& windowPosition);

    void render(); // Render the node and its ports
    void updateHoverState(const ImVec2& mousePosition); // Update hover state based on mouse position
    void updateDragState(); // Update position if the node is being dragged

    bool isHovered = false; // Expose hover state for external checks

private:
    nlohmann::json nodeData;       // Node data
    ImVec2 windowPosition;         // Position of the ImGui window
    ImVec2 position;               // Node position
    ImVec2 size;                   // Node size
    ImU32 color;                   // Node color
};
