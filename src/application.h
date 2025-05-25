#pragma once

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

/**
 * @brief 应用程序基类，负责管理窗口、OpenGL上下文和ImGui界面
 * 
 * 该类提供了基本的应用程序框架，包括：
 * - 窗口创建和管理
 * - OpenGL上下文初始化
 * - ImGui界面系统集成
 * - 主循环控制
 */
class Application {
public:
    /**
     * @brief 构造函数
     * @param name 应用程序窗口标题
     */
    Application(const char* name)
        : m_Name(name)
    {
    }

    /**
     * @brief 析构函数，负责清理所有资源
     * 
     * 清理包括：
     * - ImGui上下文和实现
     * - OpenGL资源（VAO）
     * - GLFW窗口和上下文
     */
    virtual ~Application()
    {
        // 清理 ImGui
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (m_VAO)
            glDeleteVertexArrays(1, &m_VAO);

        if (m_Window)
            glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    /**
     * @brief 创建并初始化应用程序
     * @return 如果初始化成功返回true，否则返回false
     * 
     * 初始化过程包括：
     * - GLFW初始化
     * - OpenGL上下文创建
     * - ImGui系统初始化
     * - 调用OnStart()进行派生类特定的初始化
     */
    bool Create()
    {
        if (!glfwInit())
            return false;

        // 设置 OpenGL 版本
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

        m_Window = glfwCreateWindow(1280, 720, m_Name.c_str(), nullptr, nullptr);
        if (!m_Window)
        {
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_Window);
        glfwSwapInterval(1);
        glfwShowWindow(m_Window);

        // 初始化 OpenGL
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwTerminate();
            return false;
        }

        // 创建并绑定 VAO
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        // 初始化 ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.DisplaySize = ImVec2(1280, 720);

        // 设置 ImGui 样式
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.FrameRounding = 4.0f;
        style.WindowRounding = 4.0f;
        style.GrabRounding = 3.0f;
        style.ScrollbarRounding = 3.0f;
        style.FramePadding = ImVec2(6.0f, 4.0f);
        style.ItemSpacing = ImVec2(6.0f, 4.0f);
        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.8f, 0.8f, 0.8f, 0.3f);

        // 初始化 ImGui 后端
        ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
        const char* glsl_version = "#version 330 core";
        ImGui_ImplOpenGL3_Init(glsl_version);

        OnStart();
        printf("Create\n");

        return true;
    }

    /**
     * @brief 运行应用程序主循环
     * @return 应用程序退出码
     * 
     * 主循环负责：
     * - 处理窗口事件
     * - 更新ImGui界面
     * - 渲染场景
     * - 交换缓冲区
     */
    int Run()
    {
        while (!glfwWindowShouldClose(m_Window))
        {
            glfwPollEvents();

            // 开始新帧
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            int display_w, display_h;
            glfwGetFramebufferSize(m_Window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);

            // 确保 VAO 被绑定
            glBindVertexArray(m_VAO);

            OnFrame(0.0f);

            // 渲染 ImGui
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(m_Window);
        }

        OnStop();

        return 0;
    }

protected:
    /**
     * @brief 应用程序启动时调用
     * 
     * 派生类可以重写此方法以执行特定的初始化操作
     */
    virtual void OnStart() {}

    /**
     * @brief 应用程序停止时调用
     * 
     * 派生类可以重写此方法以执行特定的清理操作
     */
    virtual void OnStop() {}

    /**
     * @brief 每帧调用
     * @param deltaTime 距离上一帧的时间间隔（秒）
     * 
     * 派生类可以重写此方法以实现具体的游戏逻辑和渲染
     */
    virtual void OnFrame(float deltaTime) {}

private:
    std::string m_Name;        ///< 应用程序窗口标题
    GLFWwindow* m_Window = nullptr;  ///< GLFW窗口句柄
    GLuint m_VAO = 0;          ///< OpenGL顶点数组对象
}; 