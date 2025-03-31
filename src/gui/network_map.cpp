#include "network_map.h"
#include "imgui.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <iostream>

void NetworkMap::render() {
    ImGui::Text("Network Map View");

    if (!networkData.empty()) {
        ImGui::Text("Loaded Network Nodes:");
        for (const auto& node : networkData) {
            ImGui::Separator();
            ImGui::Text("Node ID: %s", node["nodeID"].get<std::string>().c_str());
            ImGui::Text("Node Type ID: %d", node["nodeTypeID"].get<int>());
            ImGui::Text("Capacity: %d", node["capacity"].get<int>());

            if (node.contains("displayInfo")) {
                const auto& displayInfo = node["displayInfo"];
                ImGui::Text("Position: (%.1f, %.1f)", displayInfo["position"]["x"].get<float>(), displayInfo["position"]["y"].get<float>());
                ImGui::Text("Color: %s", displayInfo["color"].get<std::string>().c_str());
            }

            if (node.contains("inputPorts")) {
                ImGui::Text("Input Ports:");
                for (const auto& port : node["inputPorts"]) {
                    ImGui::BulletText("Port ID: %s", port["portID"].get<std::string>().c_str());
                    if (port.contains("connectedTo")) {
                        ImGui::Text("  Connected To: Node %s, Port %s",
                                    port["connectedTo"]["nodeID"].get<std::string>().c_str(),
                                    port["connectedTo"]["portID"].get<std::string>().c_str());
                    }
                }
            }

            if (node.contains("outputPorts")) {
                ImGui::Text("Output Ports:");
                for (const auto& port : node["outputPorts"]) {
                    ImGui::BulletText("Port ID: %s", port["portID"].get<std::string>().c_str());
                    if (port.contains("connectedTo")) {
                        ImGui::Text("  Connected To: Node %s, Port %s",
                                    port["connectedTo"]["nodeID"].get<std::string>().c_str(),
                                    port["connectedTo"]["portID"].get<std::string>().c_str());
                    }
                }
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
