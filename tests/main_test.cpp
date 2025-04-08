#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <imgui_node_editor.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace ed = ax::NodeEditor;

class TestContext {
public:
    TestContext() {
        // 初始化 GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // 设置 OpenGL 版本
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // 创建窗口
        m_Window = glfwCreateWindow(1280, 720, "Test Window", nullptr, nullptr);
        if (!m_Window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(m_Window);
        glfwSwapInterval(1); // 启用垂直同步

        // 初始化 OpenGL
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize OpenGL context");
        }

        // 初始化 ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.DisplaySize = ImVec2(1280, 720);

        // 设置 ImGui 样式
        ImGui::StyleColorsDark();

        // 初始化 ImGui 后端
        ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
    }

    ~TestContext() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        if (m_Window) {
            glfwDestroyWindow(m_Window);
        }
        glfwTerminate();
    }

    void NewFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Render() {
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_Window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_Window);
    }

private:
    GLFWwindow* m_Window = nullptr;
};

TEST_CASE("基本节点编辑器测试") {
    TestContext context;
    
    // 测试节点编辑器上下文创建
    ed::Config config;
    config.SettingsFile = "test.json";
    auto* editorContext = ed::CreateEditor(&config);
    
    CHECK(editorContext != nullptr);
    
    ed::DestroyEditor(editorContext);
}

TEST_CASE("节点创建测试") {
    TestContext context;
    
    ed::Config config;
    config.SettingsFile = "test.json";
    auto* editorContext = ed::CreateEditor(&config);
    
    context.NewFrame();
    
    ed::SetCurrentEditor(editorContext);
    ed::Begin("Test Editor", ImVec2(0.0, 0.0f));
    
    int uniqueId = 1;
    ed::BeginNode(uniqueId);
    ed::EndNode();
    
    ed::End();
    ed::SetCurrentEditor(nullptr);
    
    context.Render();
    
    ed::DestroyEditor(editorContext);
} 