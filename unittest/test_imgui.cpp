#include "doctest.h"
#include <GL/glew.h> // Include GLEW first
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono> // For timing

TEST_CASE("ImGui: Initialize and render a simple GUI") {
    // Initialize GLFW
    REQUIRE(glfwInit() == GLFW_TRUE);

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(800, 600, "ImGui Test", nullptr, nullptr);
    REQUIRE(window != nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader (GLEW or GLAD)
    glewExperimental = GL_TRUE; // Enable experimental features for GLEW
    REQUIRE(glewInit() == GLEW_OK);

    // Initialize ImGui context
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(800.0f, 600.0f); // Example resolution: 800x600

    // Initialize ImGui backends
    REQUIRE(ImGui_ImplGlfw_InitForOpenGL(window, true));
    REQUIRE(ImGui_ImplOpenGL3_Init("#version 130"));

    bool exitRequested = false;
    bool clicked = false;
    auto startTime = std::chrono::high_resolution_clock::now(); // Record start time

    // Main loop
    while (!glfwWindowShouldClose(window) && !exitRequested) {
        glfwPollEvents();

        // Check if 10 second has passed
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        if (elapsedTime >= 10000) {
            exitRequested = true; // Automatically request exit
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // GUI rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(display_w), static_cast<float>(display_h))); // Maximize text box
        ImGui::Begin("Hello, ImGui!", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("This is a simple GUI test. Click Exit to pass the test in 10 seconds...");
        if (ImGui::Button("Exit")) {
            exitRequested = true;
            clicked = true;
        }
        ImGui::End();

        // Render ImGui
        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
    }
    
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    CHECK_MESSAGE(clicked, "GUI test completed successfully!");
}