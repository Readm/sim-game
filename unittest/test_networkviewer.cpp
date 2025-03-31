#include <iostream> // Add this include for std::cout
// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN // Remove this line
#include "doctest.h"
#include "network_viewer.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GL/glew.h> // Add GLEW for OpenGL extensions
#include <GLFW/glfw3.h> // Add GLFW for OpenGL context
#include <chrono> // Add this include for std::chrono

TEST_CASE("NetworkViewer default mode") {
    // Initialize GLFW
    REQUIRE(glfwInit() == GLFW_TRUE);

    // Create a visible GLFW window for debugging
    GLFWwindow* window = glfwCreateWindow(800, 600, "ImGui Test", nullptr, nullptr);
    REQUIRE(window != nullptr);
    glfwShowWindow(window); // Make the window visible for debugging
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader (GLEW or GLAD)
    glewExperimental = GL_TRUE; // Enable experimental features for GLEW
    REQUIRE(glewInit() == GLEW_OK);

    // Initialize ImGui context
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(800.0f, 600.0f); // Example resolution: 800x600

    // Build font atlas
    unsigned char* font_pixels;
    int font_width, font_height;
    io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height); // Ensure font atlas is built

    // Initialize ImGui backends
    REQUIRE(ImGui_ImplGlfw_InitForOpenGL(window, true));
    REQUIRE(ImGui_ImplOpenGL3_Init("#version 130"));

    NetworkViewer viewer;

    SUBCASE("Initial mode is NetworkMap") {
        glfwPollEvents();
        // Start a new ImGui frame
        ImGui::NewFrame();

        // Simulate rendering and check the initial mode
        viewer.render();
        ImGui::Render(); // Ensure Render is called for the first frame

        // Debug log: Print the initial mode
        std::cout << "Initial mode: " << static_cast<int>(viewer.getCurrentMode()) << std::endl;

        // Render loop
        while (!viewer.shouldQuit()) {
            glfwPollEvents(); // Process events
            // Start ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Render the viewer
            viewer.render();

            // Render ImGui
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window); // Swap buffers to display the frame
        }

        // End the ImGui frame
        ImGui::Render();

        CHECK(viewer.getCurrentMode() == ViewMode::NetworkMap);
    }

    // Cleanup ImGui context
    ImGui::DestroyContext();

    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}
