#include <doctest/doctest.h>
#include "test_utils.h"
#include <imgui.h>
#include <imgui_node_editor.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace sim {
namespace gui {
namespace test {

TEST_SUITE("TestEnvironment") {
    TEST_CASE("初始化测试") {
        TestEnvironment env;
    }

    TEST_CASE("渲染测试") {
        TestEnvironment env;
        NodeEditor editor;
        REQUIRE(editor.Initialize());
        REQUIRE_NOTHROW(RenderOneFrame(editor));
    }
}

TestEnvironment::TestEnvironment() {
    std::cout << "DEBUG: Initializing test environment" << std::endl;
    
    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }
    std::cout << "DEBUG: GLFW initialized successfully" << std::endl;

    // 创建窗口
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);  // 默认隐藏窗口
    m_Window = glfwCreateWindow(800, 600, "GUI Test", nullptr, nullptr);
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    std::cout << "DEBUG: GLFW window created successfully" << std::endl;

    // 显示窗口
    glfwShowWindow(m_Window);
    glfwMakeContextCurrent(m_Window);

    // 初始化ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    std::cout << "DEBUG: ImGui context created" << std::endl;

    // 初始化ImGui后端
    if (!ImGui_ImplGlfw_InitForOpenGL(m_Window, true)) {
        std::cerr << "Failed to initialize ImGui GLFW backend" << std::endl;
        return;
    }
    std::cout << "DEBUG: ImGui GLFW backend initialized" << std::endl;

    if (!ImGui_ImplOpenGL3_Init("#version 130")) {
        std::cerr << "Failed to initialize ImGui OpenGL backend" << std::endl;
        return;
    }
    std::cout << "DEBUG: ImGui OpenGL backend initialized" << std::endl;

    // 设置ImGui样式
    ImGui::StyleColorsDark();
    std::cout << "DEBUG: ImGui style set" << std::endl;

    // 开始新帧
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    std::cout << "DEBUG: ImGui new frame started" << std::endl;

    // 初始化NodeEditor
    if (!m_Editor.Initialize()) {
        std::cerr << "Failed to initialize NodeEditor" << std::endl;
        return;
    }
    std::cout << "DEBUG: NodeEditor initialized successfully" << std::endl;

    // 结束帧
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    std::cout << "DEBUG: ImGui frame ended" << std::endl;
}

TestEnvironment::~TestEnvironment() {
    std::cout << "DEBUG: Cleaning up test environment" << std::endl;
    
    // 清理ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // 清理GLFW
    if (m_Window) {
        glfwDestroyWindow(m_Window);
    }
    glfwTerminate();
}

void TestEnvironment::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void TestEnvironment::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_Window);
}

void TestEnvironment::ShowVisualForSeconds(float seconds) {
    if (!m_Window) return;

    double startTime = glfwGetTime();
    while (glfwGetTime() - startTime < seconds) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}

} // namespace test
} // namespace gui
} // namespace sim 