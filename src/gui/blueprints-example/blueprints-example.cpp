#define IMGUI_DEFINE_MATH_OPERATORS

// 调试宏定义
#define DEBUG_SERVER 1      // 服务器相关调试信息
#define DEBUG_NODE 1        // 节点状态调试信息
#define DEBUG_EDITOR 1      // 编辑器调试信息
#define DEBUG_JSON 1        // JSON相关调试信息
#define DEBUG_ERROR 1       // 错误信息

// 定义调试宏
#define SERVER_LOG(fmt, ...) if (DEBUG_SERVER) printf("[Server] " fmt "\n", ##__VA_ARGS__)
#define NODE_LOG(fmt, ...) if (DEBUG_NODE) printf("[Node] " fmt "\n", ##__VA_ARGS__)
#define EDITOR_LOG(fmt, ...) if (DEBUG_EDITOR) printf("[Editor] " fmt "\n", ##__VA_ARGS__)
#define JSON_LOG(fmt, ...) if (DEBUG_JSON) printf("[JSON] " fmt "\n", ##__VA_ARGS__)
#define ERROR_LOG(fmt, ...) if (DEBUG_ERROR) fprintf(stderr, "[Error] " fmt "\n", ##__VA_ARGS__)

#include <application.h>
#include "utilities/builders.h"
#include "utilities/widgets.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <httplib.h>  // 添加HTTP客户端库
#include <atomic>
#include <thread>
#include <chrono>

// 添加命名空间使用声明
using json = nlohmann::json;

#include <imgui_node_editor.h>
#include <imgui_internal.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include "type.h"

static inline ImRect ImGui_GetItemRect()
{
    return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

static inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y)
{
    auto result = rect;
    result.Min.x -= x;
    result.Min.y -= y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;

using namespace ax;

using ax::Widgets::IconType;

static ed::EditorContext* m_Editor = nullptr;

//extern "C" __declspec(dllimport) short __stdcall GetAsyncKeyState(int vkey);
//extern "C" bool Debug_KeyPress(int vkey)
//{
//    static std::map<int, bool> state;
//    auto lastState = state[vkey];
//    state[vkey] = (GetAsyncKeyState(vkey) & 0x8000) != 0;
//    if (state[vkey] && !lastState)
//        return true;
//    else
//        return false;
//}

enum class PinType
{
    Flow,
    Bool,
    Int,
    Float,
    String,
    Object,
    Function,
    Delegate,
    SimPort
};

enum class PinKind
{
    Output,
    Input
};

enum class NodeType
{
    Blueprint,
    Simple,
    Tree,
    Comment,
    Houdini,
    SimNode
};

struct Node;

struct Pin
{
    ed::PinId   ID;
    ::Node*     Node;
    std::string Name;
    PinType     Type;
    PinKind     Kind;
    sim::TypeID TypeID;
    
    // 添加容量相关字段
    int capacity = 100;  // 默认容量
    int usedPackets = 0; // 当前使用的包数量

    Pin(int id, const char* name, PinType type):
        ID(id), Node(nullptr), Name(name), Type(type), Kind(PinKind::Input)
    {
    }
};

struct Node
{
    ed::NodeId ID;
    std::string Name;
    std::vector<Pin> Inputs;
    std::vector<Pin> Outputs;
    ImColor Color;
    NodeType Type;
    ImVec2 Size;

    std::string State;
    std::string SavedState;
    
    // 添加节点数据字段，用于存储节点的JSON数据
    json nodeData;

    Node(int id, const char* name, ImColor color = ImColor(255, 255, 255)):
        ID(id), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0)
    {
    }
};

struct Link
{
    ed::LinkId ID;

    ed::PinId StartPinID;
    ed::PinId EndPinID;

    ImColor Color;

    Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId):
        ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
    {
    }
};

struct NodeIdLess
{
    bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const
    {
        return lhs.AsPointer() < rhs.AsPointer();
    }
};

static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
{
    using namespace ImGui;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
    return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

struct Example:
    public Application
{
    using Application::Application;

    int GetNextId()
    {
        return m_NextId++;
    }

    //ed::NodeId GetNextNodeId()
    //{
    //    return ed::NodeId(GetNextId());
    //}

    ed::LinkId GetNextLinkId()
    {
        return ed::LinkId(GetNextId());
    }

    void TouchNode(ed::NodeId id)
    {
        m_NodeTouchTime[id] = m_TouchTime;
    }

    float GetTouchProgress(ed::NodeId id)
    {
        auto it = m_NodeTouchTime.find(id);
        if (it != m_NodeTouchTime.end() && it->second > 0.0f)
            return (m_TouchTime - it->second) / m_TouchTime;
        else
            return 0.0f;
    }

    void UpdateTouch()
    {
        const auto deltaTime = ImGui::GetIO().DeltaTime;
        for (auto& entry : m_NodeTouchTime)
        {
            if (entry.second > 0.0f)
                entry.second -= deltaTime;
        }
    }

    Node* FindNode(ed::NodeId id)
    {
        for (auto& node : m_Nodes)
            if (node.ID == id)
                return &node;

        return nullptr;
    }

    Link* FindLink(ed::LinkId id)
    {
        for (auto& link : m_Links)
            if (link.ID == id)
                return &link;

        return nullptr;
    }

    Pin* FindPin(ed::PinId id)
    {
        if (!id)
            return nullptr;

        for (auto& node : m_Nodes)
        {
            for (auto& pin : node.Inputs)
                if (pin.ID == id)
                    return &pin;

            for (auto& pin : node.Outputs)
                if (pin.ID == id)
                    return &pin;
        }

        return nullptr;
    }

    bool IsPinLinked(ed::PinId id)
    {
        if (!id)
            return false;

        for (auto& link : m_Links)
            if (link.StartPinID == id || link.EndPinID == id)
                return true;

        return false;
    }

    std::pair<bool, std::string> CanCreateLink(Pin* a, Pin* b)
    {
        if (!a || !b) {
            return {false, "Connection failed: One or both pins are null"};
        }
        if (a == b) {
            return {false, "Connection failed: Cannot connect to the same pin"};
        }
        if (a->Kind == b->Kind) {
            return {false, "Connection failed: Pins have the same direction (" + std::string(a->Kind == PinKind::Input ? "input" : "output") + ")"};
        }
        if (a->Type != b->Type) {
            return {false, "Connection failed: Type mismatch (" + std::to_string(static_cast<int>(a->Type)) + " vs " + std::to_string(static_cast<int>(b->Type)) + ")"};
        }
        // if (a->Node == b->Node) {
        //     return {false, "Connection failed: Pins belong to the same node"};
        // }

        // For SimPort type, also check if TypeID matches
        if (a->Type == PinType::SimPort && a->TypeID != b->TypeID) {
            return {false, "Connection failed: SimPort TypeID mismatch"};
        }

        return {true, ""};
    }

    //void DrawItemRect(ImColor color, float expand = 0.0f)
    //{
    //    ImGui::GetWindowDrawList()->AddRect(
    //        ImGui::GetItemRectMin() - ImVec2(expand, expand),
    //        ImGui::GetItemRectMax() + ImVec2(expand, expand),
    //        color);
    //};

    //void FillItemRect(ImColor color, float expand = 0.0f, float rounding = 0.0f)
    //{
    //    ImGui::GetWindowDrawList()->AddRectFilled(
    //        ImGui::GetItemRectMin() - ImVec2(expand, expand),
    //        ImGui::GetItemRectMax() + ImVec2(expand, expand),
    //        color, rounding);
    //};

    void BuildNode(Node* node)
    {
        for (auto& input : node->Inputs)
        {
            input.Node = node;
            input.Kind = PinKind::Input;
        }

        for (auto& output : node->Outputs)
        {
            output.Node = node;
            output.Kind = PinKind::Output;
        }
    }

    Node* SpawnInputActionNode()
    {
        m_Nodes.emplace_back(GetNextId(), "InputAction Fire", ImColor(255, 128, 128));
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Delegate);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "Pressed", PinType::Flow);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "Released", PinType::Flow);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node* SpawnBranchNode()
    {
        m_Nodes.emplace_back(GetNextId(), "Branch");
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "True", PinType::Flow);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "False", PinType::Flow);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node* SpawnDoNNode()
    {
        m_Nodes.emplace_back(GetNextId(), "Do N");
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Enter", PinType::Flow);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "N", PinType::Int);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Reset", PinType::Flow);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "Exit", PinType::Flow);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "Counter", PinType::Int);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node* SpawnOutputActionNode()
    {
        m_Nodes.emplace_back(GetNextId(), "OutputAction");
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Sample", PinType::Float);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Event", PinType::Delegate);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node* SpawnPrintStringNode()
    {
        m_Nodes.emplace_back(GetNextId(), "Print String");
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "In String", PinType::String);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node* SpawnMessageNode()
    {
        m_Nodes.emplace_back(GetNextId(), "", ImColor(128, 195, 248));
        m_Nodes.back().Type = NodeType::Simple;
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "Message", PinType::String);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node* SpawnSetTimerNode()
    {
        m_Nodes.emplace_back(GetNextId(), "Set Timer", ImColor(128, 195, 248));
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Object", PinType::Object);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Function Name", PinType::Function);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Time", PinType::Float);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Looping", PinType::Bool);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    // 保留原有的无参版本
    Node* SpawnSimNode()
    {
        m_Nodes.emplace_back(GetNextId(), "SimNode", ImColor(128, 195, 248));
        m_Nodes.back().Type = NodeType::SimNode;
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::SimPort);
        m_Nodes.back().Inputs.back().TypeID = 0;
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::SimPort);
        m_Nodes.back().Outputs.back().TypeID = 1;

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    // 新增带JSON参数的版本
    Node* SpawnSimNode(const nlohmann::json& j)
    {
        // 先创建基础节点
        m_Nodes.emplace_back(GetNextId(), "SimNode", ImColor(128, 195, 248));
        auto& node = m_Nodes.back();
        node.Type = NodeType::SimNode;

        try {
            // 解析输入端口
            if (j.contains("input_ports")) {
                for (auto& [name, port] : j["input_ports"].items()) {
                    node.Inputs.emplace_back(GetNextId(), name.c_str(), PinType::SimPort);
                    node.Inputs.back().Kind = PinKind::Input;
                    node.Inputs.back().TypeID = port["accepted_type_id"].get<sim::TypeID>();
                }
            }

            // 解析输出端口
            if (j.contains("output_ports")) {
                for (auto& [name, port] : j["output_ports"].items()) {
                    printf("hi");
                    node.Outputs.emplace_back(GetNextId(), name.c_str(), PinType::SimPort);
                    node.Outputs.back().Kind = PinKind::Output;
                    node.Outputs.back().TypeID = port["accepted_type_id"].get<sim::TypeID>();
                }
            }

            // 设置默认端口（如果没有任何端口）
            if (node.Inputs.empty()) {
                node.Inputs.emplace_back(GetNextId(), "NoInput", PinType::SimPort);
                node.Inputs.back().TypeID = 0;
            }
            if (node.Outputs.empty()) {
                node.Outputs.emplace_back(GetNextId(), "NoOutput", PinType::SimPort);
                node.Outputs.back().TypeID = 1;
            }

            BuildNode(&node);
        } catch (const std::exception& e) {
            // 异常处理：移除无效节点
            m_Nodes.pop_back();
            std::cerr << "Error deserializing SimNode: " << e.what() << std::endl;
            return nullptr;
        }

        return &node;
    }

    Node* SpawnLessNode()
    {
        m_Nodes.emplace_back(GetNextId(), "<", ImColor(128, 195, 248));
        m_Nodes.back().Type = NodeType::Simple;
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node* SpawnWeirdNode()
    {
        m_Nodes.emplace_back(GetNextId(), "o.O", ImColor(128, 195, 248));
        m_Nodes.back().Type = NodeType::Simple;
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node* SpawnTraceByChannelNode()
    {
        m_Nodes.emplace_back(GetNextId(), "Single Line Trace by Channel", ImColor(255, 128, 64));
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Start", PinType::Flow);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "End", PinType::Int);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Trace Channel", PinType::Float);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Trace Complex", PinType::Bool);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Actors to Ignore", PinType::Int);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Draw Debug Type", PinType::Bool);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Ignore Self", PinType::Bool);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "Out Hit", PinType::Float);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "Return Value", PinType::Bool);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node* SpawnTreeSequenceNode()
    {
        m_Nodes.emplace_back(GetNextId(), "Sequence");
        m_Nodes.back().Type = NodeType::Tree;
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node* SpawnTreeTaskNode()
    {
        m_Nodes.emplace_back(GetNextId(), "Move To");
        m_Nodes.back().Type = NodeType::Tree;
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node* SpawnTreeTask2Node()
    {
        m_Nodes.emplace_back(GetNextId(), "Random Wait");
        m_Nodes.back().Type = NodeType::Tree;
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node* SpawnComment()
    {
        m_Nodes.emplace_back(GetNextId(), "Test Comment");
        m_Nodes.back().Type = NodeType::Comment;
        m_Nodes.back().Size = ImVec2(300, 200);

        return &m_Nodes.back();
    }

    Node* SpawnHoudiniTransformNode()
    {
        m_Nodes.emplace_back(GetNextId(), "Transform");
        m_Nodes.back().Type = NodeType::Houdini;
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node* SpawnHoudiniGroupNode()
    {
        m_Nodes.emplace_back(GetNextId(), "Group");
        m_Nodes.back().Type = NodeType::Houdini;
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    void BuildNodes()
    {
        for (auto& node : m_Nodes)
            BuildNode(&node);
    }

    // 服务器连接相关变量
    httplib::Client* m_ServerClient = nullptr;
    std::atomic<bool> m_ServerConnected{false};
    std::atomic<bool> m_NetworkLoaded{false};
    std::string m_ServerAddress = "localhost";
    int m_ServerPort = 8080;
    std::string m_ServerStatus = "Not Connected";
    std::string m_LastError = "";
    
    // 用于异步轮询服务器状态的线程
    std::thread m_ServerPollThread;
    std::atomic<bool> m_PollThreadRunning{false};
    json m_LastNetworkState;
    
    // 连接到服务器
    bool ConnectToServer(const std::string& address, int port) {
        try {
            if (m_ServerClient) {
                delete m_ServerClient;
            }
            
            m_ServerAddress = address;
            m_ServerPort = port;
            
            m_ServerClient = new httplib::Client(m_ServerAddress, m_ServerPort);
            m_ServerClient->set_connection_timeout(3);  // 3秒超时
            m_ServerClient->set_read_timeout(3);
            
            // 测试连接
            auto res = m_ServerClient->Get("/api/health");
            if (res && res->status == 200) {
                m_ServerConnected = true;
                m_ServerStatus = "Connected";
                
                // 启动异步轮询线程
                StartPollingThread();
                
                return true;
            } else {
                m_LastError = "Connection failed: Server not responding";
                m_ServerConnected = false;
                m_ServerStatus = "Connection Failed";
                return false;
            }
        } catch (const std::exception& e) {
            m_LastError = std::string("Connection error: ") + e.what();
            m_ServerConnected = false;
            m_ServerStatus = "Connection Error";
            return false;
        }
    }
    
    // 断开服务器连接
    void DisconnectFromServer() {
        StopPollingThread();
        
        if (m_ServerClient) {
            delete m_ServerClient;
            m_ServerClient = nullptr;
        }
        
        m_ServerConnected = false;
        m_NetworkLoaded = false;
        m_ServerStatus = "Disconnected";
    }
    
    // 启动状态轮询线程
    void StartPollingThread() {
        StopPollingThread();  // 确保旧线程已停止
        
        m_PollThreadRunning = true;
        m_ServerPollThread = std::thread([this]() {
            while (m_PollThreadRunning && m_ServerConnected) {
                // 获取网络状态
                FetchNetworkState();
                
                // 暂停1秒，避免过度请求
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        });
    }
    
    // 停止状态轮询线程
    void StopPollingThread() {
        m_PollThreadRunning = false;
        
        if (m_ServerPollThread.joinable()) {
            m_ServerPollThread.join();
        }
    }
    
    // 获取网络状态
    bool FetchNetworkState() {
        if (!m_ServerConnected || !m_ServerClient) {
            SERVER_LOG("未连接到服务器");
            return false;
        }
        
        try {
            SERVER_LOG("正在获取网络状态...");
            auto res = m_ServerClient->Get("/api/network/state");
            if (res && res->status == 200) {
                SERVER_LOG("成功获取网络状态");
                // 处理可能的被引号包裹的JSON字符串
                std::string raw_body = res->body;
                
                if (raw_body.front() == '"' && raw_body.back() == '"') {
                    // 移除外部引号
                    raw_body = raw_body.substr(1, raw_body.length() - 2);
                    
                    // 处理转义字符
                    std::string::size_type pos = 0;
                    while ((pos = raw_body.find("\\\"", pos)) != std::string::npos) {
                        raw_body.replace(pos, 2, "\"");
                        pos += 1;
                    }
                    
                    // 处理其他转义字符
                    pos = 0;
                    while ((pos = raw_body.find("\\\\", pos)) != std::string::npos) {
                        raw_body.replace(pos, 2, "\\");
                        pos += 1;
                    }
                }
                
                // 解析JSON
                json newState = json::parse(raw_body);
                
                // 如果状态有变化，则更新图形
                if (m_LastNetworkState != newState) {
                    SERVER_LOG("检测到网络状态变化，更新图形");
                    m_LastNetworkState = newState;
                    m_NetworkLoaded = true;
                    
                    // 更新图形但不改变网络状态
                    UpdateGraphicsFromState();
                }
                
                return true;
            } else {
                SERVER_LOG("获取网络状态失败: HTTP %d", res ? res->status : 0);
            }
        } catch (const std::exception& e) {
            SERVER_LOG("获取网络状态异常: %s", e.what());
            m_LastError = std::string("获取网络状态异常: ") + e.what();
            return false;
        }
        
        return false;
    }
    
    // 仅更新图形显示，不改变网络状态
    void UpdateGraphicsFromState() {
        if (!m_NetworkLoaded) {
            return;
        }
        
        try {
            // 保存当前节点的位置信息
            std::map<std::string, ImVec2> nodePositions;
            for (const auto& node : m_Nodes) {
                nodePositions[node.Name] = ed::GetNodePosition(node.ID);
            }
            
            // 清空当前的节点和连接
            m_Nodes.clear();
            m_Links.clear();
            
            // 递归构建节点，保持原有位置
            BuildNodesFromJson(m_LastNetworkState, ImVec2(0, 0), 0, nodePositions);
            
            // 构建节点结构
            BuildNodes();
            
            printf("[Server] 图形更新完成\n");
        } catch (const std::exception& e) {
            m_LastError = std::string("更新图形异常: ") + e.what();
            printf("[Server] 更新图形异常: %s\n", e.what());
        }
    }
    
    // 创建生产者-消费者网络
    bool CreateProducerConsumerNetwork() {
        if (!m_ServerConnected || !m_ServerClient) {
            return false;
        }
        
        try {
            auto res = m_ServerClient->Post("/api/network/create/producer-consumer");
            if (res && res->status == 200) {
                // 获取最新网络状态
                FetchNetworkState();
                return true;
            }
        } catch (const std::exception& e) {
            m_LastError = std::string("创建网络异常: ") + e.what();
            return false;
        }
        
        return false;
    }
    
    // 启动模拟
    bool StartSimulation() {
        if (!m_ServerConnected || !m_ServerClient) {
            return false;
        }
        
        try {
            auto res = m_ServerClient->Post("/api/simulation/start");
            if (res && res->status == 200) {
                return true;
            }
        } catch (const std::exception& e) {
            m_LastError = std::string("启动模拟异常: ") + e.what();
            return false;
        }
        
        return false;
    }
    
    // 停止模拟
    bool StopSimulation() {
        if (!m_ServerConnected || !m_ServerClient) {
            return false;
        }
        
        try {
            auto res = m_ServerClient->Post("/api/simulation/stop");
            if (res && res->status == 200) {
                return true;
            }
        } catch (const std::exception& e) {
            m_LastError = std::string("停止模拟异常: ") + e.what();
            return false;
        }
        
        return false;
    }

    // 从网络状态JSON创建仿真节点
    void CreateSimNodesFromNetworkState() {
        if (!m_NetworkLoaded) {
            return;
        }
        
        // 清空当前的节点和连接
        m_Nodes.clear();
        m_Links.clear();
        
        try {
            // 递归构建节点
            BuildNodesFromJson(m_LastNetworkState, ImVec2(0, 0));
            
            // 构建节点结构
            BuildNodes();
            
            // 通知编辑器重新布局
            ed::NavigateToContent();
        } catch (const std::exception& e) {
            m_LastError = std::string("创建节点异常: ") + e.what();
        }
    }
    
    // 递归地从JSON构建节点
    Node* BuildNodesFromJson(const json& nodeJson, ImVec2 position, int depth = 0, 
                            const std::map<std::string, ImVec2>& nodePositions = std::map<std::string, ImVec2>()) {
        if (!nodeJson.is_object()) {
            return nullptr;
        }
        
        // 创建节点
        std::string nodeName = nodeJson.contains("name") ? nodeJson["name"].get<std::string>() : "SimNode";
        m_Nodes.emplace_back(GetNextId(), nodeName.c_str(), ImColor(128, 195, 248));
        auto& node = m_Nodes.back();
        node.Type = NodeType::SimNode;
        
        // 存储节点JSON数据
        node.nodeData = nodeJson;
        
        // 设置节点位置
        float yPos = position.y;  // 提前声明yPos
        auto positionIt = nodePositions.find(nodeName);
        if (positionIt != nodePositions.end()) {
            ed::SetNodePosition(node.ID, positionIt->second);
        } else {
            float xOffset = depth * 300.0f;
            ed::SetNodePosition(node.ID, ImVec2(position.x + xOffset, yPos));
        }
        
        // 节点ID
        int nodeId = 0;
        if (nodeJson.contains("node_id")) {
            nodeId = nodeJson["node_id"].get<int>();
        }
        
        // 解析输入端口
        if (nodeJson.contains("input_ports") && nodeJson["input_ports"].is_object()) {
            for (auto& [name, port] : nodeJson["input_ports"].items()) {
                node.Inputs.emplace_back(GetNextId(), name.c_str(), PinType::SimPort);
                
                // 获取端口类型ID
                if (port.contains("accepted_type_id")) {
                    node.Inputs.back().TypeID = port["accepted_type_id"].get<sim::TypeID>();
                }
                
                // 获取端口容量信息
                if (port.contains("capacity")) {
                    node.Inputs.back().capacity = port["capacity"].get<int>();
                }
                if (port.contains("packets") && port["packets"].is_array()) {
                    node.Inputs.back().usedPackets = port["packets"].size();
                }
            }
        }
        
        // 解析输出端口
        if (nodeJson.contains("output_ports") && nodeJson["output_ports"].is_object()) {
            for (auto& [name, port] : nodeJson["output_ports"].items()) {
                node.Outputs.emplace_back(GetNextId(), name.c_str(), PinType::SimPort);
                
                // 获取端口类型ID
                if (port.contains("accepted_type_id")) {
                    node.Outputs.back().TypeID = port["accepted_type_id"].get<sim::TypeID>();
                }
                
                // 获取端口容量信息
                if (port.contains("capacity")) {
                    node.Outputs.back().capacity = port["capacity"].get<int>();
                }
                if (port.contains("packets") && port["packets"].is_array()) {
                    node.Outputs.back().usedPackets = port["packets"].size();
                }
            }
        }
        
        // 设置默认端口（如果没有任何端口）
        if (node.Inputs.empty()) {
            node.Inputs.emplace_back(GetNextId(), "NoInput", PinType::SimPort);
            node.Inputs.back().TypeID = 0;
        }
        if (node.Outputs.empty()) {
            node.Outputs.emplace_back(GetNextId(), "NoOutput", PinType::SimPort);
            node.Outputs.back().TypeID = 1;
        }
        
        BuildNode(&node);
        
        // 处理子节点
        if (nodeJson.contains("children") && nodeJson["children"].is_array()) {
            float childYOffset = 0;
            int childIndex = 0;
            
            for (auto& childJson : nodeJson["children"]) {
                // 计算子节点位置
                float childY = yPos + 200.0f + childYOffset;
                childYOffset += 250.0f;  // 垂直间隔
                
                // 递归构建子节点
                auto childNode = BuildNodesFromJson(childJson, ImVec2(position.x, childY), depth + 1, nodePositions);
                
                // 如果子节点成功创建，添加连接
                if (childNode) {
                    // 从父节点到子节点的连接
                    if (!node.Outputs.empty() && !childNode->Inputs.empty()) {
                        m_Links.emplace_back(Link(GetNextLinkId(), node.Outputs[childIndex % node.Outputs.size()].ID, 
                                                 childNode->Inputs[0].ID));
                    }
                    
                    childIndex++;
                }
            }
        }
        
        return &node;
    }

    void OnStart() override
    {
        ed::Config config;

        config.SettingsFile = "Blueprints.json";
        config.UserPointer = this;

        // 暂时注释掉平滑缩放配置
        // config.EnableSmoothZoom = true;  // 启用平滑缩放

        config.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t
        {
            auto self = static_cast<Example*>(userPointer);

            auto node = self->FindNode(nodeId);
            if (!node)
            {
                NODE_LOG("尝试加载不存在的节点: %p", nodeId.AsPointer());
                return 0;
            }

            NODE_LOG("正在加载节点 %p 的状态, 状态大小: %zu", nodeId.AsPointer(), node->State.size());
            if (data != nullptr)
            {
                memcpy(data, node->State.data(), node->State.size());
                NODE_LOG("节点 %p 状态数据已复制", nodeId.AsPointer());
            }
            return node->State.size();
        };

        config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
        {
            auto self = static_cast<Example*>(userPointer);

            auto node = self->FindNode(nodeId);
            if (!node)
            {
                NODE_LOG("尝试保存不存在的节点: %p", nodeId.AsPointer());
                return false;
            }

            NODE_LOG("正在保存节点 %p 的状态, 状态大小: %zu", nodeId.AsPointer(), size);
            node->State.assign(data, size);
            NODE_LOG("节点 %p 状态已更新", nodeId.AsPointer());

            self->TouchNode(nodeId);

            return true;
        };

        m_Editor = ed::CreateEditor(&config);
        ed::SetCurrentEditor(m_Editor);

        // 检查编辑器初始化
        if (!m_Editor) {
            ERROR_LOG("编辑器初始化失败");
            return;
        }
        EDITOR_LOG("编辑器初始化成功");

        // 从JSON文件加载节点
        try {
            std::ifstream file("data/gui_node_test.json");
            JSON_LOG("正在打开gui_node_test.json");
            if (file.is_open()) {
                nlohmann::json j;
                file >> j;
                JSON_LOG("已加载JSON内容:\n%s", j.dump(4).c_str());
                
                // 清空现有的节点和连接
                m_Nodes.clear();
                m_Links.clear();
                
                // 构建节点
                BuildNodesFromJson(j, ImVec2(0, 0));
                
                // 构建节点结构
                BuildNodes();
                
                // 通知编辑器重新布局
                ed::NavigateToContent();
                
                JSON_LOG("节点加载完成");
            } else {
                ERROR_LOG("无法打开gui_node_test.json文件");
            }
        } catch (const std::exception& e) {
            ERROR_LOG("加载gui_node_test.json失败: %s", e.what());
        }

        m_HeaderBackground = LoadTexture("data/BlueprintBackground.png");
        m_SaveIcon         = LoadTexture("data/ic_save_white_24dp.png");
        m_RestoreIcon      = LoadTexture("data/ic_restore_white_24dp.png");
    }

    void OnStop() override
    {
        auto releaseTexture = [this](ImTextureID& id)
        {
            if (id)
            {
                DestroyTexture(id);
                id = nullptr;
            }
        };

        releaseTexture(m_RestoreIcon);
        releaseTexture(m_SaveIcon);
        releaseTexture(m_HeaderBackground);

        if (m_Editor)
        {
            ed::DestroyEditor(m_Editor);
            m_Editor = nullptr;
        }

        // 停止轮询线程
        StopPollingThread();
        
        // 断开服务器连接
        DisconnectFromServer();
    }

    ImColor GetIconColor(PinType type)
    {
        switch (type)
        {
            default:
            case PinType::Flow:     return ImColor(255, 255, 255);
            case PinType::Bool:     return ImColor(220,  48,  48);
            case PinType::Int:      return ImColor( 68, 201, 156);
            case PinType::Float:    return ImColor(147, 226,  74);
            case PinType::String:   return ImColor(124,  21, 153);
            case PinType::Object:   return ImColor( 51, 150, 215);
            case PinType::Function: return ImColor(218,   0, 183);
            case PinType::Delegate: return ImColor(255,  48,  48);
            case PinType::SimPort:  return ImColor(255, 100, 100);

        }
    };

    void DrawPinIcon(const Pin& pin, bool connected, int alpha)
    {
        IconType iconType;
        ImColor  color = GetIconColor(pin.Type);
        color.Value.w = alpha / 255.0f;
        switch (pin.Type)
        {
            case PinType::Flow:     iconType = IconType::Flow;   break;
            case PinType::Bool:     iconType = IconType::Circle; break;
            case PinType::Int:      iconType = IconType::Circle; break;
            case PinType::Float:    iconType = IconType::Circle; break;
            case PinType::String:   iconType = IconType::Circle; break;
            case PinType::Object:   iconType = IconType::Circle; break;
            case PinType::Function: iconType = IconType::Circle; break;
            case PinType::Delegate: iconType = IconType::Square; break;
            case PinType::SimPort:  iconType = IconType::Circle; break;
            default:
                return;
        }

        ax::Widgets::Icon(ImVec2(static_cast<float>(m_PinIconSize), static_cast<float>(m_PinIconSize)), iconType, connected, color, ImColor(32, 32, 32, alpha));
    };

    void ShowStyleEditor(bool* show = nullptr)
    {
        if (!ImGui::Begin("Style", show))
        {
            ImGui::End();
            return;
        }

        auto paneWidth = ImGui::GetContentRegionAvail().x;

        auto& editorStyle = ed::GetStyle();
        ImGui::BeginHorizontal("Style buttons", ImVec2(paneWidth, 0), 1.0f);
        ImGui::TextUnformatted("Values");
        ImGui::Spring();
        if (ImGui::Button("Reset to defaults"))
            editorStyle = ed::Style();
        ImGui::EndHorizontal();
        ImGui::Spacing();
        ImGui::DragFloat4("Node Padding", &editorStyle.NodePadding.x, 0.1f, 0.0f, 40.0f);
        ImGui::DragFloat("Node Rounding", &editorStyle.NodeRounding, 0.1f, 0.0f, 40.0f);
        ImGui::DragFloat("Node Border Width", &editorStyle.NodeBorderWidth, 0.1f, 0.0f, 15.0f);
        ImGui::DragFloat("Hovered Node Border Width", &editorStyle.HoveredNodeBorderWidth, 0.1f, 0.0f, 15.0f);
        ImGui::DragFloat("Hovered Node Border Offset", &editorStyle.HoverNodeBorderOffset, 0.1f, -40.0f, 40.0f);
        ImGui::DragFloat("Selected Node Border Width", &editorStyle.SelectedNodeBorderWidth, 0.1f, 0.0f, 15.0f);
        ImGui::DragFloat("Selected Node Border Offset", &editorStyle.SelectedNodeBorderOffset, 0.1f, -40.0f, 40.0f);
        ImGui::DragFloat("Pin Rounding", &editorStyle.PinRounding, 0.1f, 0.0f, 40.0f);
        ImGui::DragFloat("Pin Border Width", &editorStyle.PinBorderWidth, 0.1f, 0.0f, 15.0f);
        ImGui::DragFloat("Link Strength", &editorStyle.LinkStrength, 1.0f, 0.0f, 500.0f);
        //ImVec2  SourceDirection;
        //ImVec2  TargetDirection;
        ImGui::DragFloat("Scroll Duration", &editorStyle.ScrollDuration, 0.001f, 0.0f, 2.0f);
        ImGui::DragFloat("Flow Marker Distance", &editorStyle.FlowMarkerDistance, 1.0f, 1.0f, 200.0f);
        ImGui::DragFloat("Flow Speed", &editorStyle.FlowSpeed, 1.0f, 1.0f, 2000.0f);
        ImGui::DragFloat("Flow Duration", &editorStyle.FlowDuration, 0.001f, 0.0f, 5.0f);
        //ImVec2  PivotAlignment;
        //ImVec2  PivotSize;
        //ImVec2  PivotScale;
        //float   PinCorners;
        //float   PinRadius;
        //float   PinArrowSize;
        //float   PinArrowWidth;
        ImGui::DragFloat("Group Rounding", &editorStyle.GroupRounding, 0.1f, 0.0f, 40.0f);
        ImGui::DragFloat("Group Border Width", &editorStyle.GroupBorderWidth, 0.1f, 0.0f, 15.0f);

        ImGui::Separator();

        static ImGuiColorEditFlags edit_mode = ImGuiColorEditFlags_DisplayRGB;
        ImGui::BeginHorizontal("Color Mode", ImVec2(paneWidth, 0), 1.0f);
        ImGui::TextUnformatted("Filter Colors");
        ImGui::Spring();
        ImGui::RadioButton("RGB", &edit_mode, ImGuiColorEditFlags_DisplayRGB);
        ImGui::Spring(0);
        ImGui::RadioButton("HSV", &edit_mode, ImGuiColorEditFlags_DisplayHSV);
        ImGui::Spring(0);
        ImGui::RadioButton("HEX", &edit_mode, ImGuiColorEditFlags_DisplayHex);
        ImGui::EndHorizontal();

        static ImGuiTextFilter filter;
        filter.Draw("##filter", paneWidth);

        ImGui::Spacing();

        ImGui::PushItemWidth(-160);
        for (int i = 0; i < ed::StyleColor_Count; ++i)
        {
            auto name = ed::GetStyleColorName((ed::StyleColor)i);
            if (!filter.PassFilter(name))
                continue;

            ImGui::ColorEdit4(name, &editorStyle.Colors[i].x, edit_mode);
        }
        ImGui::PopItemWidth();

        ImGui::End();
    }

    void ShowLeftPane(float paneWidth)
    {
        auto& io = ImGui::GetIO();

        ImGui::BeginChild("Selection", ImVec2(paneWidth, 0));

        paneWidth = ImGui::GetContentRegionAvail().x;

        // 添加服务器控制面板到左侧面板的底部
        if (ImGui::CollapsingHeader("Server Control", ImGuiTreeNodeFlags_DefaultOpen))
        {
            char addressBuffer[128] = {0};
            strncpy(addressBuffer, m_ServerAddress.c_str(), sizeof(addressBuffer) - 1);
            
            ImGui::PushItemWidth(paneWidth * 0.6f);
            if (ImGui::InputText("Address", addressBuffer, sizeof(addressBuffer))) {
                m_ServerAddress = addressBuffer;
            }
            
            ImGui::SameLine();
            
            char portBuffer[16] = {0};
            snprintf(portBuffer, sizeof(portBuffer), "%d", m_ServerPort);
            
            ImGui::PushItemWidth(paneWidth * 0.2f);
            if (ImGui::InputText("Port", portBuffer, sizeof(portBuffer), ImGuiInputTextFlags_CharsDecimal)) {
                m_ServerPort = std::atoi(portBuffer);
            }
            
            ImGui::Text("Status: %s", m_ServerStatus.c_str());
            
            if (!m_ServerConnected) {
                if (ImGui::Button("Connect Server", ImVec2(paneWidth, 0))) {
                    ConnectToServer(m_ServerAddress, m_ServerPort);
                }
            } else {
                if (ImGui::Button("Disconnect", ImVec2(paneWidth * 0.48f, 0))) {
                    DisconnectFromServer();
                }
                
                ImGui::Separator();
                
                if (ImGui::Button("Producer-Consumer Network", ImVec2(paneWidth, 0))) {
                    CreateProducerConsumerNetwork();
                }
                
                if (ImGui::Button("Start Simulation", ImVec2(paneWidth * 0.48f, 0))) {
                    StartSimulation();
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop Simulation", ImVec2(paneWidth * 0.48f, 0))) {
                    StopSimulation();
                }
                
                if (!m_LastError.empty()) {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", m_LastError.c_str());
                }
                
                if (m_NetworkLoaded) {
                    ImGui::Separator();
                    ImGui::TextUnformatted("Network Info");
                    
                    bool running = false;
                    int tick = 0;
                    
                    if (m_LastNetworkState.contains("running")) {
                        running = m_LastNetworkState["running"].get<bool>();
                    }
                    
                    if (m_LastNetworkState.contains("tick_tock")) {
                        tick = m_LastNetworkState["tick_tock"].get<int>();
                    }
                    
                    ImGui::Text("Running State: %s", running ? "Running" : "Stopped");
                    ImGui::Text("Current Tick: %d", tick);
                    
                    int nodeCount = 0;
                    if (m_LastNetworkState.contains("children") && m_LastNetworkState["children"].is_array()) {
                        nodeCount = m_LastNetworkState["children"].size();
                    }
                    
                    ImGui::Text("Node Count: %d", nodeCount);
                }
            }
        }
        
        // 添加网络状态JSON显示区
        if (m_NetworkLoaded && ImGui::CollapsingHeader("Network State JSON", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // JSON内容可能很长，所以使用一个带滚动条的区域
            ImGui::BeginChild("JSONContent", ImVec2(0, 300), true);
            
            // 添加一个复制按钮
            if (ImGui::Button("复制到剪贴板")) {
                ImGui::SetClipboardText(m_LastNetworkState.dump(2).c_str());
            }
            
            // 使用格式化的JSON显示
            ImGui::TextWrapped("%s", m_LastNetworkState.dump(2).c_str());
            
            ImGui::EndChild();
        }

        static bool showStyleEditor = false;
        ImGui::BeginHorizontal("Style Editor", ImVec2(paneWidth, 0));
        ImGui::Spring(0.0f, 0.0f);
        if (ImGui::Button("Zoom to Content"))
            ed::NavigateToContent();
        ImGui::Spring(0.0f);
        if (ImGui::Button("Show Flow"))
        {
            for (auto& link : m_Links)
                ed::Flow(link.ID);
        }
        ImGui::Spring();
        if (ImGui::Button("Edit Style"))
            showStyleEditor = true;
        ImGui::EndHorizontal();
        ImGui::Checkbox("Show Ordinals", &m_ShowOrdinals);

        if (showStyleEditor)
            ShowStyleEditor(&showStyleEditor);

        std::vector<ed::NodeId> selectedNodes;
        std::vector<ed::LinkId> selectedLinks;
        selectedNodes.resize(ed::GetSelectedObjectCount());
        selectedLinks.resize(ed::GetSelectedObjectCount());

        int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
        int linkCount = ed::GetSelectedLinks(selectedLinks.data(), static_cast<int>(selectedLinks.size()));

        selectedNodes.resize(nodeCount);
        selectedLinks.resize(linkCount);

        int saveIconWidth     = GetTextureWidth(m_SaveIcon);
        int saveIconHeight    = GetTextureWidth(m_SaveIcon);
        int restoreIconWidth  = GetTextureWidth(m_RestoreIcon);
        int restoreIconHeight = GetTextureWidth(m_RestoreIcon);

        ImGui::GetWindowDrawList()->AddRectFilled(
            ImGui::GetCursorScreenPos(),
            ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
            ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
        ImGui::Spacing(); ImGui::SameLine();
        ImGui::TextUnformatted("Nodes");
        ImGui::Indent();
        for (auto& node : m_Nodes)
        {
            ImGui::PushID(node.ID.AsPointer());
            auto start = ImGui::GetCursorScreenPos();

            if (const auto progress = GetTouchProgress(node.ID))
            {
                ImGui::GetWindowDrawList()->AddLine(
                    start + ImVec2(-8, 0),
                    start + ImVec2(-8, ImGui::GetTextLineHeight()),
                    IM_COL32(255, 0, 0, 255 - (int)(255 * progress)), 4.0f);
            }

            bool isSelected = std::find(selectedNodes.begin(), selectedNodes.end(), node.ID) != selectedNodes.end();
# if IMGUI_VERSION_NUM >= 18967
            ImGui::SetNextItemAllowOverlap();
# endif
            if (ImGui::Selectable((node.Name + "##" + std::to_string(reinterpret_cast<uintptr_t>(node.ID.AsPointer()))).c_str(), &isSelected))
            {
                if (io.KeyCtrl)
                {
                    if (isSelected)
                        ed::SelectNode(node.ID, true);
                    else
                        ed::DeselectNode(node.ID);
                }
                else
                    ed::SelectNode(node.ID, false);

                ed::NavigateToSelection();
            }
            if (ImGui::IsItemHovered() && !node.State.empty())
                ImGui::SetTooltip("State: %s", node.State.c_str());

            auto id = std::string("(") + std::to_string(reinterpret_cast<uintptr_t>(node.ID.AsPointer())) + ")";
            auto textSize = ImGui::CalcTextSize(id.c_str(), nullptr);
            auto iconPanelPos = start + ImVec2(
                paneWidth - ImGui::GetStyle().FramePadding.x - ImGui::GetStyle().IndentSpacing - saveIconWidth - restoreIconWidth - ImGui::GetStyle().ItemInnerSpacing.x * 1,
                (ImGui::GetTextLineHeight() - saveIconHeight) / 2);
            ImGui::GetWindowDrawList()->AddText(
                ImVec2(iconPanelPos.x - textSize.x - ImGui::GetStyle().ItemInnerSpacing.x, start.y),
                IM_COL32(255, 255, 255, 255), id.c_str(), nullptr);

            auto drawList = ImGui::GetWindowDrawList();
            ImGui::SetCursorScreenPos(iconPanelPos);
# if IMGUI_VERSION_NUM < 18967
            ImGui::SetItemAllowOverlap();
# else
            ImGui::SetNextItemAllowOverlap();
# endif
            if (node.SavedState.empty())
            {
                if (ImGui::InvisibleButton("save", ImVec2((float)saveIconWidth, (float)saveIconHeight)))
                    node.SavedState = node.State;

                if (ImGui::IsItemActive())
                    drawList->AddImage(m_SaveIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 96));
                else if (ImGui::IsItemHovered())
                    drawList->AddImage(m_SaveIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
                else
                    drawList->AddImage(m_SaveIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 160));
            }
            else
            {
                ImGui::Dummy(ImVec2((float)saveIconWidth, (float)saveIconHeight));
                drawList->AddImage(m_SaveIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 32));
            }

            ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
# if IMGUI_VERSION_NUM < 18967
            ImGui::SetItemAllowOverlap();
# else
            ImGui::SetNextItemAllowOverlap();
# endif
            if (!node.SavedState.empty())
            {
                if (ImGui::InvisibleButton("restore", ImVec2((float)restoreIconWidth, (float)restoreIconHeight)))
                {
                    node.State = node.SavedState;
                    ed::RestoreNodeState(node.ID);
                    node.SavedState.clear();
                }

                if (ImGui::IsItemActive())
                    drawList->AddImage(m_RestoreIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 96));
                else if (ImGui::IsItemHovered())
                    drawList->AddImage(m_RestoreIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
                else
                    drawList->AddImage(m_RestoreIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 160));
            }
            else
            {
                ImGui::Dummy(ImVec2((float)restoreIconWidth, (float)restoreIconHeight));
                drawList->AddImage(m_RestoreIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 32));
            }

            ImGui::SameLine(0, 0);
# if IMGUI_VERSION_NUM < 18967
            ImGui::SetItemAllowOverlap();
# endif
            ImGui::Dummy(ImVec2(0, (float)restoreIconHeight));

            ImGui::PopID();
        }
        ImGui::Unindent();

        static int changeCount = 0;

        ImGui::GetWindowDrawList()->AddRectFilled(
            ImGui::GetCursorScreenPos(),
            ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
            ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
        ImGui::Spacing(); ImGui::SameLine();
        ImGui::TextUnformatted("Selection");

        ImGui::BeginHorizontal("Selection Stats", ImVec2(paneWidth, 0));
        ImGui::Text("Changed %d time%s", changeCount, changeCount > 1 ? "s" : "");
        ImGui::Spring();
        if (ImGui::Button("Deselect All"))
            ed::ClearSelection();
        ImGui::EndHorizontal();
        ImGui::Indent();
        for (int i = 0; i < nodeCount; ++i) ImGui::Text("Node (%p)", selectedNodes[i].AsPointer());
        for (int i = 0; i < linkCount; ++i) ImGui::Text("Link (%p)", selectedLinks[i].AsPointer());
        ImGui::Unindent();

        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
            for (auto& link : m_Links)
                ed::Flow(link.ID);

        if (ed::HasSelectionChanged())
            ++changeCount;

        ImGui::EndChild();
    }

    void OnFrame(float deltaTime) override
    {
        UpdateTouch();
        auto& io = ImGui::GetIO();
        
        // 将编辑器上下文设置移到函数开始处
        ed::SetCurrentEditor(m_Editor);
        
        ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

        static bool lastMiddleButtonState = false;
        if (io.MouseDown[ImGuiMouseButton_Middle] != lastMiddleButtonState) {
            lastMiddleButtonState = io.MouseDown[ImGuiMouseButton_Middle];
        }
        
        // 检查鼠标移动
        static ImVec2 lastMousePos = ImVec2(0, 0);
        if (io.MousePos.x != lastMousePos.x || io.MousePos.y != lastMousePos.y) {
            lastMousePos = io.MousePos;
        }

        // 恢复必要的变量定义
        static ed::NodeId contextNodeId = 0;
        static ed::LinkId contextLinkId = 0;
        static ed::PinId contextPinId = 0;
        static bool createNewNode = false;
        static Pin* newNodeLinkPin = nullptr;
        static Pin* newLinkPin = nullptr;

        static float leftPaneWidth = 400.0f;
        static float rightPaneWidth = 800.0f;
        Splitter(true, 4.0f, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);

        ShowLeftPane(leftPaneWidth - 4.0f);

        ImGui::SameLine(0.0f, 12.0f);

        ed::Begin("Node editor");
        {
            auto cursorTopLeft = ImGui::GetCursorScreenPos();

            util::BlueprintNodeBuilder builder(m_HeaderBackground, GetTextureWidth(m_HeaderBackground), GetTextureHeight(m_HeaderBackground));

            for (auto& node : m_Nodes)
            {
                if (node.Type != NodeType::Blueprint && node.Type != NodeType::Simple && node.Type != NodeType::SimNode)
                    continue;

                const auto isSimple = node.Type == NodeType::Simple;

                bool hasOutputDelegates = false;
                for (auto& output : node.Outputs)
                    if (output.Type == PinType::Delegate)
                        hasOutputDelegates = true;

                builder.Begin(node.ID);
                    if (!isSimple)
                    {
                        builder.Header(node.Color);
                            ImGui::Spring(0);
                            ImGui::TextUnformatted(node.Name.c_str());
                            ImGui::Spring(1);
                            ImGui::Dummy(ImVec2(0, 28));
                        builder.EndHeader();
                    }

                    // 输入端口及其进度条
                    for (auto& input : node.Inputs)
                    {
                        auto alpha = ImGui::GetStyle().Alpha;
                        auto [canCreate, _] = CanCreateLink(newLinkPin, &input);
                        if (newLinkPin && !canCreate && &input != newLinkPin)
                            alpha = alpha * (48.0f / 255.0f);

                        builder.Input(input.ID);
                        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                        DrawPinIcon(input, IsPinLinked(input.ID), (int)(alpha * 255));
                        ImGui::Spring(0);
                        if (!input.Name.empty())
                        {
                            ImGui::TextUnformatted(input.Name.c_str());
                            ImGui::Spring(0);
                        }
                        
                        // 为SimPort类型添加进度条
                        if (input.Type == PinType::SimPort)
                        {
                            // 计算占用率
                            float fraction = input.capacity > 0 ? static_cast<float>(input.usedPackets) / input.capacity : 0.0f;
                            
                            // 容量显示的文本
                            char overlay[32];
                            snprintf(overlay, sizeof(overlay), "%d/%d", input.usedPackets, input.capacity);
                            
                            // 根据占用率变化颜色
                            ImVec4 progressColor;
                            if (fraction < 0.5f) {
                                // 绿色到黄色的渐变
                                progressColor = ImVec4(fraction * 2.0f, 1.0f, 0.0f, 1.0f);
                            } else {
                                // 黄色到红色的渐变
                                progressColor = ImVec4(1.0f, 2.0f * (1.0f - fraction), 0.0f, 1.0f);
                            }
                            
                            // 保存当前颜色
                            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImGui::ColorConvertFloat4ToU32(progressColor));
                            
                            // 获取文字高度
                            float textHeight = ImGui::GetTextLineHeight();
                            
                            // 添加容量进度条
                            ImGui::ProgressBar(fraction, ImVec2(100, textHeight), overlay);
                            
                            // 恢复颜色
                            ImGui::PopStyleColor();
                        }
                        
                        if (input.Type == PinType::Bool)
                        {
                             ImGui::Button("Hello");
                             ImGui::Spring(0);
                        }
                        ImGui::PopStyleVar();
                        builder.EndInput();
                    }

                    // 为SimNode类型添加节点总容量进度条
                    if (node.Type == NodeType::SimNode)
                    {
                        builder.Middle();

                        // 计算节点总容量
                        int totalCapacity = 0;
                        int totalUsed = 0;
                        
                        // 计算输入端口的总容量和使用情况
                        for (auto& input : node.Inputs) {
                            if (input.Type == PinType::SimPort) {
                                totalCapacity += input.capacity;
                                totalUsed += input.usedPackets;
                            }
                        }
                        
                        // 计算输出端口的总容量和使用情况
                        for (auto& output : node.Outputs) {
                            if (output.Type == PinType::SimPort) {
                                totalCapacity += output.capacity;
                                totalUsed += output.usedPackets;
                            }
                        }
                        
                        // 如果有容量，显示节点总进度条
                        if (totalCapacity > 0) {
                            // 计算占用率
                            float fraction = static_cast<float>(totalUsed) / totalCapacity;
                            
                            // 容量显示的文本
                            char overlay[32];
                            snprintf(overlay, sizeof(overlay), "%d/%d", totalUsed, totalCapacity);
                            
                            // 根据占用率变化颜色
                            ImVec4 progressColor;
                            if (fraction < 0.5f) {
                                // 绿色到黄色的渐变
                                progressColor = ImVec4(fraction * 2.0f, 1.0f, 0.0f, 1.0f);
                            } else {
                                // 黄色到红色的渐变
                                progressColor = ImVec4(1.0f, 2.0f * (1.0f - fraction), 0.0f, 1.0f);
                            }
                            
                            // 保存当前颜色
                            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImGui::ColorConvertFloat4ToU32(progressColor));
                            
                            // 添加容量进度条 - 居中显示
                            ImGui::Spring(1, 0);
                            float textHeight = ImGui::GetTextLineHeight() * 1.8f;  // 从1.2f改为1.8f，增加50%的高度
                            ImGui::ProgressBar(fraction, ImVec2(100, textHeight), overlay);
                            ImGui::Spring(1, 0);
                            
                            // 恢复颜色
                            ImGui::PopStyleColor();
                        }
                    }

                    if (isSimple)
                    {
                        builder.Middle();

                        ImGui::Spring(1, 0);
                        ImGui::TextUnformatted(node.Name.c_str());
                        ImGui::Spring(1, 0);
                    }

                    // 输出端口及其进度条
                    for (auto& output : node.Outputs)
                    {
                        if (!isSimple && output.Type == PinType::Delegate)
                            continue;

                        auto alpha = ImGui::GetStyle().Alpha;
                        auto [canCreate, _] = CanCreateLink(newLinkPin, &output);
                        if (newLinkPin && !canCreate && &output != newLinkPin)
                            alpha = alpha * (48.0f / 255.0f);

                        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                        builder.Output(output.ID);
                        
                        // 为SimPort类型添加进度条
                        if (output.Type == PinType::SimPort)
                        {
                            // 计算占用率
                            float fraction = output.capacity > 0 ? static_cast<float>(output.usedPackets) / output.capacity : 0.0f;
                            
                            // 容量显示的文本
                            char overlay[32];
                            snprintf(overlay, sizeof(overlay), "%d/%d", output.usedPackets, output.capacity);
                            
                            // 根据占用率变化颜色
                            ImVec4 progressColor;
                            if (fraction < 0.5f) {
                                // 绿色到黄色的渐变
                                progressColor = ImVec4(fraction * 2.0f, 1.0f, 0.0f, 1.0f);
                            } else {
                                // 黄色到红色的渐变
                                progressColor = ImVec4(1.0f, 2.0f * (1.0f - fraction), 0.0f, 1.0f);
                            }
                            
                            // 保存当前颜色
                            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImGui::ColorConvertFloat4ToU32(progressColor));
                            
                            // 获取文字高度
                            float textHeight = ImGui::GetTextLineHeight();
                            
                            // 添加容量进度条
                            ImGui::ProgressBar(fraction, ImVec2(100, textHeight), overlay);
                            
                            // 恢复颜色
                            ImGui::PopStyleColor();
                        }
                        
                        if (output.Type == PinType::String)
                        {
                            static char buffer[128] = "Edit Me\nMultiline!";
                            static bool wasActive = false;

                            ImGui::PushItemWidth(100.0f);
                            ImGui::InputText("##edit", buffer, 127);
                            ImGui::PopItemWidth();
                            if (ImGui::IsItemActive() && !wasActive)
                            {
                                ed::EnableShortcuts(false);
                                wasActive = true;
                            }
                            else if (!ImGui::IsItemActive() && wasActive)
                            {
                                ed::EnableShortcuts(true);
                                wasActive = false;
                            }
                            ImGui::Spring(0);
                        }
                        if (!output.Name.empty())
                        {
                            ImGui::Spring(0);
                            ImGui::TextUnformatted(output.Name.c_str());
                        }
                        ImGui::Spring(0);
                        DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
                        ImGui::PopStyleVar();
                        builder.EndOutput();
                    }

                builder.End();
            }

            for (auto& link : m_Links)
                ed::Link(link.ID, link.StartPinID, link.EndPinID, link.Color, 2.0f);

            if (!createNewNode)
            {
                if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
                {
                    auto showLabel = [](const char* label, ImColor color)
                    {
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
                        auto size = ImGui::CalcTextSize(label);

                        auto padding = ImGui::GetStyle().FramePadding;
                        auto spacing = ImGui::GetStyle().ItemSpacing;

                        ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

                        auto rectMin = ImGui::GetCursorScreenPos() - padding;
                        auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

                        auto drawList = ImGui::GetWindowDrawList();
                        drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
                        ImGui::TextUnformatted(label);
                    };

                    ed::PinId startPinId = 0, endPinId = 0;
                    if (ed::QueryNewLink(&startPinId, &endPinId))
                    {
                        auto startPin = FindPin(startPinId);
                        auto endPin   = FindPin(endPinId);

                        newLinkPin = startPin ? startPin : endPin;

                        if (startPin->Kind == PinKind::Input)
                        {
                            std::swap(startPin, endPin);
                            std::swap(startPinId, endPinId);
                        }

                        if (startPin && endPin)
                        {
                            if (endPin == startPin)
                            {
                                showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
                                ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                            }
                            else if (endPin->Kind == startPin->Kind)
                            {
                                showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
                                ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                            }
                            else
                            {
                                auto [canCreate, errorMsg] = CanCreateLink(startPin, endPin);
                                if (!canCreate)
                                {
                                    showLabel(errorMsg.c_str(), ImColor(45, 32, 32, 180));
                                    ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                                }
                                else
                                {
                                    showLabel("+ Create Link", ImColor(32, 45, 32, 180));
                                    if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                                    {
                                        m_Links.emplace_back(Link(GetNextId(), startPinId, endPinId));
                                        m_Links.back().Color = GetIconColor(startPin->Type);
                                    }
                                }
                            }
                        }
                    }

                    ed::PinId pinId = 0;
                    if (ed::QueryNewNode(&pinId))
                    {
                        newLinkPin = FindPin(pinId);
                        if (newLinkPin)
                            showLabel("+ Create Node", ImColor(32, 45, 32, 180));

                        if (ed::AcceptNewItem())
                        {
                            createNewNode  = true;
                            newNodeLinkPin = FindPin(pinId);
                            newLinkPin = nullptr;
                            ed::Suspend();
                            ImGui::OpenPopup("Create New Node");
                            ed::Resume();
                        }
                    }
                }
                else
                    newLinkPin = nullptr;

                ed::EndCreate();

                if (ed::BeginDelete())
                {
                    ed::NodeId nodeId = 0;
                    while (ed::QueryDeletedNode(&nodeId))
                    {
                        if (ed::AcceptDeletedItem())
                        {
                            auto id = std::find_if(m_Nodes.begin(), m_Nodes.end(), [nodeId](auto& node) { return node.ID == nodeId; });
                            if (id != m_Nodes.end())
                                m_Nodes.erase(id);
                        }
                    }

                    ed::LinkId linkId = 0;
                    while (ed::QueryDeletedLink(&linkId))
                    {
                        if (ed::AcceptDeletedItem())
                        {
                            auto id = std::find_if(m_Links.begin(), m_Links.end(), [linkId](auto& link) { return link.ID == linkId; });
                            if (id != m_Links.end())
                                m_Links.erase(id);
                        }
                    }
                }
                ed::EndDelete();
            }

            ImGui::SetCursorScreenPos(cursorTopLeft);
        }

    # if 1
        auto openPopupPosition = ImGui::GetMousePos();
        ed::Suspend();
        if (ed::ShowNodeContextMenu(&contextNodeId))
            ImGui::OpenPopup("Node Context Menu");
        else if (ed::ShowPinContextMenu(&contextPinId))
            ImGui::OpenPopup("Pin Context Menu");
        else if (ed::ShowLinkContextMenu(&contextLinkId))
            ImGui::OpenPopup("Link Context Menu");
        else if (ed::ShowBackgroundContextMenu())
        {
            ImGui::OpenPopup("Create New Node");
            newNodeLinkPin = nullptr;
        }
        ed::Resume();

        // 添加一个变量来存储要显示JSON的节点
        static Node* showJsonNode = nullptr;
        static bool showJsonWindow = false;

        ed::Suspend();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        if (ImGui::BeginPopup("Node Context Menu"))
        {
            auto node = FindNode(contextNodeId);

            ImGui::TextUnformatted("Node Context Menu");
            ImGui::Separator();
            if (node)
            {
                ImGui::Text("ID: %p", node->ID.AsPointer());
                ImGui::Text("Type: %s", node->Type == NodeType::Blueprint ? "Blueprint" : (node->Type == NodeType::Tree ? "Tree" : "Comment"));
                ImGui::Text("Inputs: %d", (int)node->Inputs.size());
                ImGui::Text("Outputs: %d", (int)node->Outputs.size());
                
                // 如果是SimNode类型，添加Show JSON选项
                if (node->Type == NodeType::SimNode)
                {
                    ImGui::Separator();
                    if (ImGui::MenuItem("Show JSON"))
                    {
                        showJsonNode = node;
                        showJsonWindow = true;
                    }
                }
            }
            else
                ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
            ImGui::Separator();
            if (ImGui::MenuItem("Delete"))
                ed::DeleteNode(contextNodeId);
            ImGui::EndPopup();
        }

        // 如果需要显示JSON窗口
        if (showJsonWindow && showJsonNode)
        {
            ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
            char windowTitle[128];
            snprintf(windowTitle, sizeof(windowTitle), "JSON Data: %s###NodeJSON", showJsonNode->Name.c_str());
            
            if (ImGui::Begin(windowTitle, &showJsonWindow))
            {
                if (!showJsonNode->nodeData.empty())
                {
                    // 添加复制按钮
                    if (ImGui::Button("Copy to Clipboard"))
                    {
                        ImGui::SetClipboardText(showJsonNode->nodeData.dump(2).c_str());
                    }
                    
                    ImGui::SameLine();
                    if (ImGui::Button("Close"))
                    {
                        showJsonWindow = false;
                    }
                    
                    // 在滚动窗口中显示格式化的JSON
                    ImGui::BeginChild("JSONScrollRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
                    
                    ImGui::TextUnformatted(showJsonNode->nodeData.dump(2).c_str());
                    
                    ImGui::EndChild();
                }
                else
                {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "节点没有JSON数据");
                    if (ImGui::Button("关闭"))
                    {
                        showJsonWindow = false;
                    }
                }
            }
            ImGui::End();
            
            // 如果窗口关闭了，清除节点引用
            if (!showJsonWindow)
                showJsonNode = nullptr;
        }

        if (ImGui::BeginPopup("Pin Context Menu"))
        {
            auto pin = FindPin(contextPinId);

            ImGui::TextUnformatted("Pin Context Menu");
            ImGui::Separator();
            if (pin)
            {
                ImGui::Text("ID: %p", pin->ID.AsPointer());
                if (pin->Node)
                    ImGui::Text("Node: %p", pin->Node->ID.AsPointer());
                else
                    ImGui::Text("Node: %s", "<none>");
            }
            else
                ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());

            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("Link Context Menu"))
        {
            auto link = FindLink(contextLinkId);

            ImGui::TextUnformatted("Link Context Menu");
            ImGui::Separator();
            if (link)
            {
                ImGui::Text("ID: %p", link->ID.AsPointer());
                ImGui::Text("From: %p", link->StartPinID.AsPointer());
                ImGui::Text("To: %p", link->EndPinID.AsPointer());
            }
            else
                ImGui::Text("Unknown link: %p", contextLinkId.AsPointer());
            ImGui::Separator();
            if (ImGui::MenuItem("Delete"))
                ed::DeleteLink(contextLinkId);
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("Create New Node"))
        {
            auto newNodePostion = openPopupPosition;
            //ImGui::SetCursorScreenPos(ImGui::GetMousePosOnOpeningCurrentPopup());

            //auto drawList = ImGui::GetWindowDrawList();
            //drawList->AddCircleFilled(ImGui::GetMousePosOnOpeningCurrentPopup(), 10.0f, 0xFFFF00FF);

            Node* node = nullptr;
            if (ImGui::MenuItem("Input Action"))
                node = SpawnInputActionNode();
            if (ImGui::MenuItem("Output Action"))
                node = SpawnOutputActionNode();
            if (ImGui::MenuItem("Branch"))
                node = SpawnBranchNode();
            if (ImGui::MenuItem("Do N"))
                node = SpawnDoNNode();
            if (ImGui::MenuItem("Set Timer"))
                node = SpawnSetTimerNode();
            if (ImGui::MenuItem("Less"))
                node = SpawnLessNode();
            if (ImGui::MenuItem("Weird"))
                node = SpawnWeirdNode();
            if (ImGui::MenuItem("Trace by Channel"))
                node = SpawnTraceByChannelNode();
            if (ImGui::MenuItem("Print String"))
                node = SpawnPrintStringNode();
            ImGui::Separator();
            if (ImGui::MenuItem("Comment"))
                node = SpawnComment();
            ImGui::Separator();
            if (ImGui::MenuItem("Sequence"))
                node = SpawnTreeSequenceNode();
            if (ImGui::MenuItem("Move To"))
                node = SpawnTreeTaskNode();
            if (ImGui::MenuItem("Random Wait"))
                node = SpawnTreeTask2Node();
            ImGui::Separator();
            if (ImGui::MenuItem("Message"))
                node = SpawnMessageNode();
            ImGui::Separator();
            if (ImGui::MenuItem("Transform"))
                node = SpawnHoudiniTransformNode();
            if (ImGui::MenuItem("Group"))
                node = SpawnHoudiniGroupNode();

            if (node)
            {
                BuildNodes();

                createNewNode = false;

                ed::SetNodePosition(node->ID, newNodePostion);

                if (auto startPin = newNodeLinkPin)
                {
                    auto& pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;

                    for (auto& pin : pins)
                    {
                        auto [canCreate, _] = CanCreateLink(startPin, &pin);
                        if (canCreate)
                        {
                            auto endPin = &pin;
                            if (startPin->Kind == PinKind::Input)
                                std::swap(startPin, endPin);

                            m_Links.emplace_back(Link(GetNextId(), startPin->ID, endPin->ID));
                            m_Links.back().Color = GetIconColor(startPin->Type);

                            break;
                        }
                    }
                }
            }

            ImGui::EndPopup();
        }
        else
            createNewNode = false;
        ImGui::PopStyleVar();
        ed::Resume();
    # endif


    /*
        cubic_bezier_t c;
        c.p0 = pointf(100, 600);
        c.p1 = pointf(300, 1200);
        c.p2 = pointf(500, 100);
        c.p3 = pointf(900, 600);

        auto drawList = ImGui::GetWindowDrawList();
        auto offset_radius = 15.0f;
        auto acceptPoint = [drawList, offset_radius](const bezier_subdivide_result_t& r)
        {
            drawList->AddCircle(to_imvec(r.point), 4.0f, IM_COL32(255, 0, 255, 255));

            auto nt = r.tangent.normalized();
            nt = pointf(-nt.y, nt.x);

            drawList->AddLine(to_imvec(r.point), to_imvec(r.point + nt * offset_radius), IM_COL32(255, 0, 0, 255), 1.0f);
        };

        drawList->AddBezierCurve(to_imvec(c.p0), to_imvec(c.p1), to_imvec(c.p2), to_imvec(c.p3), IM_COL32(255, 255, 255, 255), 1.0f);
        cubic_bezier_subdivide(acceptPoint, c);
    */

        ed::End();

        auto editorMin = ImGui::GetItemRectMin();
        auto editorMax = ImGui::GetItemRectMax();

        if (m_ShowOrdinals)
        {
            int nodeCount = ed::GetNodeCount();
            std::vector<ed::NodeId> orderedNodeIds;
            orderedNodeIds.resize(static_cast<size_t>(nodeCount));
            ed::GetOrderedNodeIds(orderedNodeIds.data(), nodeCount);


            auto drawList = ImGui::GetWindowDrawList();
            drawList->PushClipRect(editorMin, editorMax);

            int ordinal = 0;
            for (auto& nodeId : orderedNodeIds)
            {
                auto p0 = ed::GetNodePosition(nodeId);
                auto p1 = p0 + ed::GetNodeSize(nodeId);
                p0 = ed::CanvasToScreen(p0);
                p1 = ed::CanvasToScreen(p1);


                ImGuiTextBuffer builder;
                builder.appendf("#%d", ordinal++);

                auto textSize   = ImGui::CalcTextSize(builder.c_str());
                auto padding    = ImVec2(2.0f, 2.0f);
                auto widgetSize = textSize + padding * 2;

                auto widgetPosition = ImVec2(p1.x, p0.y) + ImVec2(0.0f, -widgetSize.y);

                drawList->AddRectFilled(widgetPosition, widgetPosition + widgetSize, IM_COL32(100, 80, 80, 190), 3.0f, ImDrawFlags_RoundCornersAll);
                drawList->AddRect(widgetPosition, widgetPosition + widgetSize, IM_COL32(200, 160, 160, 190), 3.0f, ImDrawFlags_RoundCornersAll);
                drawList->AddText(widgetPosition + padding, IM_COL32(255, 255, 255, 255), builder.c_str());
            }

            drawList->PopClipRect();
        }


        //ImGui::ShowTestWindow();
        //ImGui::ShowMetricsWindow();
    }

    int                  m_NextId = 1;
    const int            m_PinIconSize = 24;
    std::vector<Node>    m_Nodes;
    std::vector<Link>    m_Links;
    ImTextureID          m_HeaderBackground = nullptr;
    ImTextureID          m_SaveIcon = nullptr;
    ImTextureID          m_RestoreIcon = nullptr;
    const float          m_TouchTime = 1.0f;
    std::map<ed::NodeId, float, NodeIdLess> m_NodeTouchTime;
    bool                 m_ShowOrdinals = false;
};

int Main(int argc, char** argv)
{
    Example exampe("Blueprints", argc, argv);

    if (exampe.Create())
        return exampe.Run();

    return 0;
}