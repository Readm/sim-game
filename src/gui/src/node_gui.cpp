#include "../include/node_gui.h"
#include "../include/node.h"
#include "../include/pin.h"
#include "../include/link.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

namespace sim {
namespace gui {

NodeGUI::NodeGUI() : m_Initialized(false) {
    std::cout << "DEBUG: Creating NodeGUI object" << std::endl;
}

NodeGUI::~NodeGUI() {
}

bool NodeGUI::Initialize() {
    std::cout << "DEBUG: Initializing NodeGUI" << std::endl;
    
    if (m_Initialized) {
        return true;
    }

    if (!m_NodeEditor.Initialize()) {
        return false;
    }

    m_Initialized = true;
    std::cout << "DEBUG: NodeGUI initialization completed" << std::endl;
    return true;
}

void NodeGUI::Render() {
    if (!m_Initialized) {
        return;
    }

    m_NodeEditor.Render();
}

bool NodeGUI::LoadFromJson(const std::string& jsonStr) {
    try {
        json j = json::parse(jsonStr);
        
        // 创建节点
        for (const auto& nodeJson : j["nodes"]) {
            std::string name = nodeJson["name"];
            float x = nodeJson["position"]["x"];
            float y = nodeJson["position"]["y"];
            
            auto node = m_NodeEditor.CreateNode(name, ImVec2(x, y));
            
            // 添加输入引脚
            if (nodeJson.contains("inputs")) {
                for (const auto& input : nodeJson["inputs"]) {
                    node->AddInputPin(input["name"], ed::PinKind::Input);
                }
            }
            
            // 添加输出引脚
            if (nodeJson.contains("outputs")) {
                for (const auto& output : nodeJson["outputs"]) {
                    node->AddOutputPin(output["name"], ed::PinKind::Output);
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading from JSON: " << e.what() << std::endl;
        return false;
    }
}

} // namespace gui
} // namespace sim 