#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <memory>
#include "node_ui.h"

class NetworkMap {
public:
    // Render the network map view
    void render();

    // Load network data from a JSON string
    void loadNetworkData(const std::string& jsonContent);

    // Check if any node is currently being dragged
    bool isAnyNodeDragging() const;

private:
    nlohmann::json networkData; // Stores the loaded network data
    std::unordered_map<std::string, std::unique_ptr<NodeUI>> nodeUIs;  // 存储 NodeUI 实例的映射
};
