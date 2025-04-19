#pragma once

#include <memory>
#include <vector>
#include <string>
#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace sim {
namespace gui {

class Node;
class Pin;
class Link;

class NodeEditor {
public:
    NodeEditor();
    ~NodeEditor();

    // 初始化编辑器
    bool Initialize();
    
    // 渲染编辑器
    void Render();
    
    // 创建新节点
    std::shared_ptr<Node> CreateNode(const std::string& name, const ImVec2& position);
    
    // 删除节点
    void DeleteNode(std::shared_ptr<Node> node);
    
    // 创建连接
    std::shared_ptr<Link> CreateLink(std::shared_ptr<Pin> startPin, std::shared_ptr<Pin> endPin);
    
    // 删除连接
    void DeleteLink(std::shared_ptr<Link> link);

    // 获取编辑器上下文
    ed::EditorContext* GetContext() const { return m_Context; }

    // 内部辅助函数
    void DrawNodes();
    void DrawLinks();
    void HandleNodeCreation();
    void HandleLinkCreation();
    void HandleDeletion();

private:
    static ed::EditorContext* s_SharedContext;
    ed::EditorContext* m_Context;
    std::vector<std::shared_ptr<Node>> m_Nodes;
    std::vector<std::shared_ptr<Link>> m_Links;
};

} // namespace gui
} // namespace sim 