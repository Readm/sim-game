#include "node_ui.h"
#include <iostream>
#include <stdexcept>

NodeUI::NodeUI(const nlohmann::json& node, const ImVec2& windowPos) {
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

void NodeUI::render() const {
    ImGui::Separator();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(position, ImVec2(position.x + size.x, position.y + size.y), color);
}
