#pragma once
#include <string>
#include <nlohmann/json.hpp>

class NetworkMap {
public:
    // Render the network map view
    void render();

    // Load network data from a JSON string
    void loadNetworkData(const std::string& jsonContent);

private:
    nlohmann::json networkData; // Stores the loaded network data
};
