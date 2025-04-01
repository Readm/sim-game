#include "network_map.h"
#include "node_ui.h"
#include "imgui.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <iostream>

// TODO: use static nodeUI to avoid reset position
void NetworkMap::render() {
    bool isDraggingNode = false;

    if (!networkData.empty()) {
        // Get the position of the current ImGui window
        ImVec2 windowPos = ImGui::GetWindowPos();

        for (const auto& node : networkData) {
            try {
                NodeUI nodeUI(node, windowPos);
                nodeUI.render(); // NodeUI now handles port rendering

                // Check if any node is being dragged
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && nodeUI.isHovered) {
                    isDraggingNode = true;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error rendering node: " << e.what() << std::endl;
            }
        }
    } else {
        ImGui::Text("No network data loaded.");
    }

    // Prevent window movement if a node is being dragged
    if (isDraggingNode) {
        ImGui::SetWindowPos(ImGui::GetWindowPos(), ImGuiCond_Always);
    }
}

void NetworkMap::loadNetworkData(const std::string& jsonContent) {
    try {
        networkData = nlohmann::json::parse(jsonContent);
        std::cout << "Network data successfully loaded." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON content: " << e.what() << std::endl;
    }
}
