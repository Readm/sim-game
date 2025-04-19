#pragma once

#include <memory>
#include <string>
#include <imgui.h>
#include <imgui_node_editor.h>
#include "node_editor.h"

namespace sim {
namespace gui {

class NodeGUI {
public:
    NodeGUI();
    ~NodeGUI();

    // 初始化GUI
    bool Initialize();

    // 渲染GUI
    void Render();

    // 获取节点编辑器
    NodeEditor& GetNodeEditor() { return m_NodeEditor; }

    // 从JSON加载节点
    bool LoadFromJson(const std::string& json);

private:
    NodeEditor m_NodeEditor;
    bool m_Initialized;
};

} // namespace gui
} // namespace sim 