#include "network_map.h"
#include "node_ui.h"
#include "imgui.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <iostream>

// TODO: use static nodeUI to avoid reset position
void NetworkMap::render() {
    if (!networkData.empty()) {
        ImVec2 windowPos = ImGui::GetWindowPos();

        for (const auto& node : networkData) {
            try {
                // 安全地检查和获取节点ID
                std::string nodeId;
                if (node.contains("id") && node["id"].is_string()) {
                    nodeId = node["id"].get<std::string>();
                } else {
                    // 如果没有id字段，使用节点数据的字符串表示作为临时ID
                    nodeId = node.dump();
                }
                
                // 如果这个节点还没有对应的 NodeUI，创建一个
                if (nodeUIs.find(nodeId) == nodeUIs.end()) {
                    nodeUIs[nodeId] = std::make_unique<NodeUI>(node, windowPos);
                } else {
                    // 更新现有 NodeUI 的窗口位置
                    nodeUIs[nodeId]->updateWindowPosition(windowPos);
                }

                // 渲染节点
                nodeUIs[nodeId]->render();
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
        auto newNetworkData = nlohmann::json::parse(jsonContent);
        
        // 清理不再存在的节点的 UI
        for (auto it = nodeUIs.begin(); it != nodeUIs.end();) {
            bool nodeExists = false;
            for (const auto& node : newNetworkData) {
                std::string nodeId;
                if (node.contains("id") && node["id"].is_string()) {
                    nodeId = node["id"].get<std::string>();
                } else {
                    nodeId = node.dump();
                }
                
                if (nodeId == it->first) {
                    nodeExists = true;
                    break;
                }
            }
            if (!nodeExists) {
                it = nodeUIs.erase(it);
            } else {
                ++it;
            }
        }
        
        networkData = std::move(newNetworkData);
        std::cout << "Network data successfully loaded." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON content: " << e.what() << std::endl;
    }
}

bool NetworkMap::isAnyNodeDragging() const {
    for (const auto& [nodeId, nodeUI] : nodeUIs) {
        if (nodeUI->shouldDisableWindowDrag()) {
            return true;
        }
    }
    return false;
}
