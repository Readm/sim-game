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
    void updateWindowPosition(const ImVec2& newWindowPos); // Update window position

    bool isHovered = false; // Expose hover state for external checks
    bool isDragging = false; // 新增：跟踪是否正在拖拽

    // 新增：获取是否应该禁用窗口交互
    bool shouldDisableWindowDrag() const { return isDragging; }

private:
    nlohmann::json nodeData;       // Node data
    ImVec2 windowPosition;         // Position of the ImGui window
    ImVec2 position;               // Node position
    ImVec2 size;                   // Node size
    ImU32 color;                   // Node color
};
