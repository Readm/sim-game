#include "node_ui.h"
#include <iostream>
#include <stdexcept>
#include "imgui.h"

NodeUI::NodeUI(const nlohmann::json& node, const ImVec2& windowPos)
    : nodeData(node), windowPosition(windowPos) { // Initialize nodeData and windowPosition
    if (!node.contains("displayInfo")) {
        throw std::invalid_argument("Node does not contain displayInfo");
    }

    const auto& displayInfo = node["displayInfo"];

    position = ImVec2(
        windowPos.x + displayInfo["position"]["x"].get<float>(),
        windowPos.y + displayInfo["position"]["y"].get<float>()
    );
    size = ImVec2(
        displayInfo["size"]["width"].get<float>(),
        displayInfo["size"]["height"].get<float>()
    );
    std::string colorStr = displayInfo["color"].get<std::string>();
    color = ImGui::ColorConvertFloat4ToU32(ImVec4(
        std::stoi(colorStr.substr(1, 2), nullptr, 16) / 255.0f,
        std::stoi(colorStr.substr(3, 2), nullptr, 16) / 255.0f,
        std::stoi(colorStr.substr(5, 2), nullptr, 16) / 255.0f,
        1.0f
    ));
}

void NodeUI::updateHoverState(const ImVec2& mousePosition) {
    isHovered = mousePosition.x >= position.x && mousePosition.x <= position.x + size.x &&
                mousePosition.y >= position.y && mousePosition.y <= position.y + size.y;
}

void NodeUI::updateDragState() {
    // 如果鼠标左键刚被按下且鼠标在节点上，开始拖拽
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && isHovered) {
        isDragging = true;
    }
    
    // 如果正在拖拽且鼠标左键仍然按下，更新位置
    if (isDragging && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
        position.x += dragDelta.x;
        position.y += dragDelta.y;
        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
    }
    
    // 如果鼠标左键释放，结束拖拽
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        isDragging = false;
    }
}

void NodeUI::render() {
    updateHoverState(ImGui::GetMousePos());
    updateDragState(); 
    ImGui::Separator();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Highlight the node if hovered
    ImU32 renderColor = isHovered ? IM_COL32(255, 255, 0, 255) : color;

    drawList->AddRectFilled(position, ImVec2(position.x + size.x, position.y + size.y), renderColor);

    // Draw a white border if hovered
    if (isHovered) {
        drawList->AddRect(position, ImVec2(position.x + size.x, position.y + size.y), IM_COL32(255, 255, 255, 255), 0.0f, 0, 2.0f);
    }

    // Draw input ports
    if (nodeData.contains("inputPorts")) {
        const auto& inputPorts = nodeData["inputPorts"];
        for (size_t i = 0; i < inputPorts.size(); ++i) {
            // 使用当前节点位置计算端口位置
            ImVec2 portPos(
                position.x - 10,  // 左侧端口
                position.y + (i + 1) * (size.y / (inputPorts.size() + 1))
            );
            ImGui::GetWindowDrawList()->AddCircleFilled(portPos, 5.0f, IM_COL32(255, 255, 255, 255));
        }
    }

    // Draw output ports
    if (nodeData.contains("outputPorts")) {
        const auto& outputPorts = nodeData["outputPorts"];
        for (size_t i = 0; i < outputPorts.size(); ++i) {
            // 使用当前节点位置计算端口位置
            ImVec2 portPos(
                position.x + size.x + 10,  // 右侧端口
                position.y + (i + 1) * (size.y / (outputPorts.size() + 1))
            );
            ImGui::GetWindowDrawList()->AddCircleFilled(portPos, 5.0f, IM_COL32(255, 255, 255, 255));
        }
    }
}

void NodeUI::updateWindowPosition(const ImVec2& newWindowPos) {
    // 计算相对位置的偏移
    float dx = newWindowPos.x - windowPosition.x;
    float dy = newWindowPos.y - windowPosition.y;
    
    // 更新位置
    position.x += dx;
    position.y += dy;
    
    // 保存新的窗口位置
    windowPosition = newWindowPos;
}
