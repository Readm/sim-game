#pragma once

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class Application {
public:
    Application(const char* name)
        : m_Name(name)
    {
    }

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
    virtual void OnStart() {}
    virtual void OnStop() {}
    virtual void OnFrame(float deltaTime) {}

private:
    std::string m_Name;
    GLFWwindow* m_Window = nullptr;
    GLuint m_VAO = 0;
}; 