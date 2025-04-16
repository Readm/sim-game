#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include "network_viewer.h"
#include "http_client.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int argc, char** argv)
{
    // 设置GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1280, 720, "网络仿真查看器", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // 启用垂直同步

    // 设置ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // 设置ImGui风格
    ImGui::StyleColorsDark();

    // 设置平台/渲染器后端
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // 创建网络查看器和HTTP客户端
    NetworkViewer viewer;
    HttpClient client;

    viewer.init();
    client.setStateUpdateCallback([&viewer](const std::string& state) {
        viewer.updateNetworkState(state);
    });

    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // 开始新帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 渲染网络查看器
        viewer.render();

        // 渲染
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // 清理
    viewer.shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
} 