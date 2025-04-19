#pragma once

#include <stdexcept>
#include <imgui.h>
#include <imgui_node_editor.h>
#include <GLFW/glfw3.h>
#include "../include/node_editor.h"
#include <string>

struct GLFWwindow;

namespace sim {
namespace gui {
namespace test {

// 测试环境初始化
class TestEnvironment {
public:
    TestEnvironment();
    ~TestEnvironment();

    void BeginFrame();
    void EndFrame();

    NodeEditor& GetNodeEditor() { return m_Editor; }

    // 显示可视化测试结果
    void ShowVisualForSeconds(float seconds);

private:
    GLFWwindow* m_Window;
    NodeEditor m_Editor;
};

// 模拟渲染一帧
inline void RenderOneFrame(NodeEditor& editor) {
    ImGui::NewFrame();
    editor.Render();
    ImGui::Render();
}

} // namespace test
} // namespace gui
} // namespace sim 