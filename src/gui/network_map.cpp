#include "network_map.h"
#include "node_ui.h"
#include "imgui.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <iostream>

void NetworkMap::render() {
    ImGui::Text("Network Map View");

    if (!networkData.empty()) {
        // Get the position of the current ImGui window
        ImVec2 windowPos = ImGui::GetWindowPos();

        for (const auto& node : networkData) {
            try {
                NodeUI nodeUI(node, windowPos);
                nodeUI.render();
            } catch (const std::exception& e) {
                std::cerr << "Error rendering node: " << e.what() << std::endl;
            }
        }
    } else {
        ImGui::Text("No network data loaded.");
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
