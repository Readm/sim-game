#include "node_gui.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_node_editor.h>
#include <string>
#include <sstream>
#include <iomanip>

namespace ed = ax::NodeEditor;

// 构造函数
NodeGUI::NodeGUI() 
    : editorContext_(nullptr)
    , nextConnectionId_(1)
    , selectedNodeId_(0) {
}

// 析构函数
NodeGUI::~NodeGUI() {
    shutdown();
}

// 初始化
void NodeGUI::init() {
    ed::Config config;
    config.SettingsFile = "node_editor.json"; // 保存编辑器配置的文件
    editorContext_ = ed::CreateEditor(&config);
}

// 关闭
void NodeGUI::shutdown() {
    if (editorContext_) {
        ed::DestroyEditor(editorContext_);
        editorContext_ = nullptr;
    }
}

// 渲染
void NodeGUI::render(const std::shared_ptr<sim::Node>& node) {
    if (!editorContext_ || !node) {
        return;
    }

    ed::SetCurrentEditor(editorContext_);
    ed::Begin("Node Editor");

    // 绘制节点及其子节点
    drawNode(node);
    drawChildNodes(node);

    // 处理节点选择
    if (ed::GetSelectedObjectCount() > 0) {
        // 创建一个数组来接收选中的节点ID
        ed::NodeId selectedNodeIds[16]; // 假设最多16个选中的节点
        int count = ed::GetSelectedNodes(selectedNodeIds, 16);
        
        if (count > 0) {
            const auto& nodeId = selectedNodeIds[0]; // 取第一个选中的节点
            for (const auto& [id, visual] : nodeVisuals_) {
                if (visual.editorId == nodeId) {
                    selectedNodeId_ = id;
                    if (nodeSelectedCallback_) {
                        // 此处需要根据ID查找node，这是个简化版
                        nodeSelectedCallback_(node);
                    }
                    break;
                }
            }
        }
    }

    // 处理链接创建
    if (ed::BeginCreate()) {
        ed::PinId startPinId, endPinId;
        if (ed::QueryNewLink(&startPinId, &endPinId)) {
            if (startPinId && endPinId) {
                // 查找起始和结束端口信息
                sim::NodeID startNodeId = 0, endNodeId = 0;
                std::string startPortName, endPortName;
                bool startIsInput = false, endIsInput = false;

                // 这里需要查找端口信息，实际实现需要遍历nodeVisuals_
                // 简化版实现，实际代码需要完善

                if (startNodeId != 0 && endNodeId != 0 && 
                    !startPortName.empty() && !endPortName.empty() &&
                    startIsInput != endIsInput) {
                    // 确保一个是输入另一个是输出端口
                    if (startIsInput) {
                        // 如果开始是输入，交换位置
                        std::swap(startNodeId, endNodeId);
                        std::swap(startPortName, endPortName);
                    }
                    
                    // 创建链接
                    createConnection(startNodeId, startPortName, endNodeId, endPortName);
                    ed::AcceptNewItem();
                }
            }
        }
        ed::EndCreate();
    }

    // 处理链接删除
    if (ed::BeginDelete()) {
        ed::LinkId deletedLinkId;
        while (ed::QueryDeletedLink(&deletedLinkId)) {
            for (const auto& [id, conn] : connectionVisuals_) {
                if (conn.editorId == deletedLinkId) {
                    deleteConnection(id);
                    break;
                }
            }
            ed::AcceptDeletedItem();
        }
        ed::EndDelete();
    }

    // 绘制现有连接
    for (const auto& [id, conn] : connectionVisuals_) {
        ed::Link(conn.editorId, 
                 getPinId(conn.fromNodeId, conn.fromPortName, false), 
                 getPinId(conn.toNodeId, conn.toPortName, true));
    }

    ed::End();
    ed::SetCurrentEditor(nullptr);
}

// 绘制节点
void NodeGUI::drawNode(const std::shared_ptr<sim::Node>& node) {
    if (!node) return;

    auto& visual = getOrCreateNodeVisual(node);
    
    ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(8, 8, 8, 8));
    
    ed::BeginNode(visual.editorId);
    
    // 节点标题
    ImGui::TextUnformatted(visual.name.c_str());
    ImGui::Separator();
    
    // 节点内容（类型和ID）
    ImGui::Text("Type: %s", visual.type.c_str());
    ImGui::Text("ID: %lu", node->getNodeID());  // 修正格式化字符串
    ImGui::Text("Tick: %lu", node->getTickTock());
    
    // 绘制端口
    drawPorts(node);
    
    ed::EndNode();
    
    // 记录节点位置
    if (ImGui::IsItemActive()) {
        visual.position = ed::GetNodePosition(visual.editorId);
    }
    
    ed::PopStyleVar();
}

// 绘制节点的输入输出端口
void NodeGUI::drawPorts(const std::shared_ptr<sim::Node>& node) {
    if (!node) return;
    
    auto& visual = getOrCreateNodeVisual(node);
    
    // 绘制输入端口
    ImGui::BeginGroup();
    for (const auto& [portName, port] : node->getInputPorts()) {
        ed::BeginPin(visual.inputPinIds[portName], ed::PinKind::Input);
        
        ImGui::BeginHorizontal(portName.c_str());
        ImGui::Spring(0);
        ImGui::TextUnformatted(portName.c_str());
        ImGui::Spring(1);
        ImGui::EndHorizontal();
        
        ed::EndPin();
    }
    ImGui::EndGroup();
    
    ImGui::SameLine();
    
    // 绘制输出端口
    ImGui::BeginGroup();
    for (const auto& [portName, port] : node->getOutputPorts()) {
        ed::BeginPin(visual.outputPinIds[portName], ed::PinKind::Output);
        
        ImGui::BeginHorizontal(portName.c_str());
        ImGui::Spring(1);
        ImGui::TextUnformatted(portName.c_str());
        ImGui::Spring(0);
        ImGui::EndHorizontal();
        
        ed::EndPin();
    }
    ImGui::EndGroup();
}

// 绘制节点属性面板
void NodeGUI::drawNodeProperties(const std::shared_ptr<sim::Node>& node) {
    if (!node) return;
    
    ImGui::Begin("Node Properties");
    
    ImGui::Text("Node ID: %lu", node->getNodeID());  // 修正格式化字符串
    ImGui::Text("Type ID: %lu", node->getTypeID());  // 修正格式化字符串
    ImGui::Text("Packet Type ID: %lu", node->getPacketTypeID());  // 修正格式化字符串
    ImGui::Text("Tick-Tock: %lu", node->getTickTock());
    
    // 显示缓冲区
    if (ImGui::CollapsingHeader("Buffer")) {
        const auto& buffer = node->getBuffer();
        for (size_t i = 0; i < buffer.size(); ++i) {
            const auto& packet = buffer[i];
            std::stringstream ss;
            ss << "Packet " << i << " (ID: " 
               << packet->getPacketID().node_id << ":" 
               << packet->getPacketID().local_seq << ")";
            
            if (ImGui::TreeNode(ss.str().c_str())) {
                ImGui::Text("Source Node: %lu", packet->getSrcNodeID());  // 修正格式化字符串
                ImGui::Text("Type: %lu", packet->getTypeID());  // 修正格式化字符串
                
                // 如果是InfoPacket，显示其信息
                if (auto infoPacket = std::dynamic_pointer_cast<sim::InfoPacket>(packet)) {
                    ImGui::Text("Info: %s", infoPacket->getInfo().c_str());
                }
                
                ImGui::TreePop();
            }
        }
    }
    
    // 显示输入端口
    if (ImGui::CollapsingHeader("Input Ports")) {
        for (const auto& [name, port] : node->getInputPorts()) {
            if (ImGui::TreeNode(name.c_str())) {
                ImGui::Text("Type ID: %lu", port->getAcceptedTypeID());  // 修正方法名和格式化字符串
                ImGui::Text("Capacity: %zu", port->getCapacity());
                ImGui::Text("Valid: %s", port->isValid() ? "Yes" : "No");  // 修改为可用的方法
                
                // InputPort没有getPackets方法，使用peekPacket来获取当前包
                if (port->isValid()) {
                    if (ImGui::TreeNode("Current Packet")) {
                        auto packet = port->peekPacket();
                        if (packet) {
                            ImGui::Text("Packet ID: %lu:%lu", 
                                        packet->getPacketID().node_id,
                                        packet->getPacketID().local_seq);
                        }
                        ImGui::TreePop();
                    }
                }
                
                ImGui::TreePop();
            }
        }
    }
    
    // 显示输出端口
    if (ImGui::CollapsingHeader("Output Ports")) {
        for (const auto& [name, port] : node->getOutputPorts()) {
            if (ImGui::TreeNode(name.c_str())) {
                ImGui::Text("Type ID: %lu", port->getAcceptedTypeID());  // 修正方法名和格式化字符串
                ImGui::Text("Capacity: %zu", port->getCapacity());
                ImGui::Text("Ready: %s", port->isReady() ? "Yes" : "No");
                
                // 显示连接的输入端口
                const auto& connectedPorts = port->getConnectedPorts();
                if (!connectedPorts.empty()) {
                    if (ImGui::TreeNode("Connected Ports")) {
                        for (size_t i = 0; i < connectedPorts.size(); ++i) {
                            ImGui::Text("Port %zu: %s", i, connectedPorts[i]->getName().c_str());
                        }
                        ImGui::TreePop();
                    }
                }
                
                ImGui::TreePop();
            }
        }
    }
    
    ImGui::End();
}

// 绘制子节点
void NodeGUI::drawChildNodes(const std::shared_ptr<sim::Node>& node) {
    if (!node) return;
    
    const auto& children = node->getChildren();
    for (const auto& child : children) {
        drawNode(child);
    }
}

// 设置节点位置
void NodeGUI::setNodePosition(sim::NodeID nodeId, const ImVec2& position) {
    auto it = nodeVisuals_.find(nodeId);
    if (it != nodeVisuals_.end()) {
        it->second.position = position;
        ed::SetNodePosition(it->second.editorId, position);
    }
}

// 获取节点位置
ImVec2 NodeGUI::getNodePosition(sim::NodeID nodeId) const {
    auto it = nodeVisuals_.find(nodeId);
    if (it != nodeVisuals_.end()) {
        return it->second.position;
    }
    return ImVec2(0, 0);
}

// 处理节点创建事件
void NodeGUI::handleNodeCreated(const std::shared_ptr<sim::Node>& node) {
    if (!node) return;
    
    auto& visual = getOrCreateNodeVisual(node);
    // 新创建的节点设置在一个合理的位置
    setNodePosition(node->getNodeID(), ImVec2(100.0f, 100.0f));
}

// 处理节点删除事件
void NodeGUI::handleNodeDeleted(sim::NodeID nodeId) {
    auto it = nodeVisuals_.find(nodeId);
    if (it != nodeVisuals_.end()) {
        // 删除所有相关的连接
        std::vector<int> connectionsToDelete;
        for (const auto& [id, conn] : connectionVisuals_) {
            if (conn.fromNodeId == nodeId || conn.toNodeId == nodeId) {
                connectionsToDelete.push_back(id);
            }
        }
        
        for (int id : connectionsToDelete) {
            deleteConnection(id);
        }
        
        nodeVisuals_.erase(it);
    }
    
    if (selectedNodeId_ == nodeId) {
        selectedNodeId_ = 0;
    }
}

// 处理选择变更事件
void NodeGUI::handleSelectionChanged() {
    // 已在render()中处理
}

// 创建连接
void NodeGUI::createConnection(sim::NodeID fromNodeId, const std::string& fromPortName,
                              sim::NodeID toNodeId, const std::string& toPortName) {
    // 检查节点和端口是否存在
    auto fromNodeIt = nodeVisuals_.find(fromNodeId);
    auto toNodeIt = nodeVisuals_.find(toNodeId);
    
    if (fromNodeIt == nodeVisuals_.end() || toNodeIt == nodeVisuals_.end()) {
        return;
    }
    
    auto fromPortIt = fromNodeIt->second.outputPinIds.find(fromPortName);
    auto toPortIt = toNodeIt->second.inputPinIds.find(toPortName);
    
    if (fromPortIt == fromNodeIt->second.outputPinIds.end() || 
        toPortIt == toNodeIt->second.inputPinIds.end()) {
        return;
    }
    
    // 创建连接可视化对象
    ConnectionVisual connection;
    connection.id = nextConnectionId_++;
    connection.fromNodeId = fromNodeId;
    connection.fromPortName = fromPortName;
    connection.toNodeId = toNodeId;
    connection.toPortName = toPortName;
    connection.editorId = ed::LinkId(connection.id);
    
    connectionVisuals_[connection.id] = connection;
    
    // 实际连接节点的端口逻辑需要添加在这里
    // 这里应该调用Node的接口来连接端口
}

// 删除连接
void NodeGUI::deleteConnection(int connectionId) {
    auto it = connectionVisuals_.find(connectionId);
    if (it != connectionVisuals_.end()) {
        // 实际断开节点端口连接的逻辑需要添加在这里
        // 这里应该调用Node的接口来断开端口
        
        connectionVisuals_.erase(it);
    }
}

// 获取选中的节点
std::shared_ptr<sim::Node> NodeGUI::getSelectedNode() const {
    // 这里需要根据selectedNodeId_查找实际的Node对象
    // 简化版实现，实际代码需要完善
    return nullptr;
}

// 设置节点选中回调
void NodeGUI::setNodeSelectedCallback(NodeSelectedCallback callback) {
    nodeSelectedCallback_ = callback;
}

// 获取节点颜色
ImColor NodeGUI::getNodeColor(const std::string& nodeType) const {
    // 根据节点类型返回不同的颜色
    if (nodeType == "Producer") {
        return ImColor(32, 128, 64);
    } else if (nodeType == "Consumer") {
        return ImColor(128, 32, 64);
    } else if (nodeType == "Router") {
        return ImColor(32, 64, 128);
    } else if (nodeType == "Filter") {
        return ImColor(128, 128, 32);
    }
    return ImColor(64, 64, 64);
}

// 获取端口颜色
ImColor NodeGUI::getPortColor(sim::TypeID portTypeId) const {
    // 根据端口类型返回不同的颜色
    switch (portTypeId) {
        case 1: return ImColor(220, 48, 48);  // VoidPacket
        case 2: return ImColor(48, 220, 48);  // InfoPacket
        default: return ImColor(180, 180, 180);
    }
}

// 获取端口的PinId
ed::PinId NodeGUI::getPinId(sim::NodeID nodeId, const std::string& portName, bool isInput) const {
    auto it = nodeVisuals_.find(nodeId);
    if (it != nodeVisuals_.end()) {
        const auto& visual = it->second;
        if (isInput) {
            auto pinIt = visual.inputPinIds.find(portName);
            if (pinIt != visual.inputPinIds.end()) {
                return pinIt->second;
            }
        } else {
            auto pinIt = visual.outputPinIds.find(portName);
            if (pinIt != visual.outputPinIds.end()) {
                return pinIt->second;
            }
        }
    }
    return ed::PinId(0);
}

// 获取或创建节点可视化对象
NodeGUI::NodeVisual& NodeGUI::getOrCreateNodeVisual(const std::shared_ptr<sim::Node>& node) {
    auto it = nodeVisuals_.find(node->getNodeID());
    if (it != nodeVisuals_.end()) {
        return it->second;
    }
    
    // 创建新的可视化对象
    NodeVisual visual;
    visual.id = node->getNodeID();
    visual.type = std::to_string(node->getTypeID());  // 实际项目中可能需要类型名称映射
    visual.name = "Node " + std::to_string(node->getNodeID());
    visual.position = ImVec2(100.0f + node->getNodeID() * 50.0f, 100.0f);  // 简单的位置计算
    visual.editorId = ed::NodeId(static_cast<uint64_t>(node->getNodeID()));
    
    // 创建输入端口的PinId
    for (const auto& [portName, port] : node->getInputPorts()) {
        uint64_t pinId = (static_cast<uint64_t>(node->getNodeID()) << 32) | 
                          (static_cast<uint64_t>(1) << 31) | 
                          static_cast<uint64_t>(visual.inputPinIds.size() + 1);
        visual.inputPinIds[portName] = ed::PinId(pinId);
    }
    
    // 创建输出端口的PinId
    for (const auto& [portName, port] : node->getOutputPorts()) {
        uint64_t pinId = (static_cast<uint64_t>(node->getNodeID()) << 32) | 
                          static_cast<uint64_t>(visual.outputPinIds.size() + 1);
        visual.outputPinIds[portName] = ed::PinId(pinId);
    }
    
    // 设置节点位置
    ed::SetNodePosition(visual.editorId, visual.position);
    
    nodeVisuals_[visual.id] = visual;
    return nodeVisuals_[visual.id];
}

// 更新节点布局
void NodeGUI::updateNodeVisualLayout(NodeVisual& visual) {
    // 根据需要更新节点布局
} 