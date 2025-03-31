#include "imgui.h"
#include <vector>
#include <string>

enum class ViewMode {
    NetworkMap,
    Timeline,
    TrafficChart
};

class NetworkViewer {
public:
    void render() {
        renderModeSelector();
        switch (currentMode) {
            case ViewMode::NetworkMap:
                renderNetworkMap();
                break;
            case ViewMode::Timeline:
                renderTimeline();
                break;
            case ViewMode::TrafficChart:
                renderTrafficChart();
                break;
        }
    }

    ViewMode getCurrentMode() const {
        return currentMode;
    }

private:
    ViewMode currentMode = ViewMode::NetworkMap;

    void renderModeSelector() {
        if (ImGui::Begin("Mode Selector")) {
            if (ImGui::RadioButton("Network Map", currentMode == ViewMode::NetworkMap)) {
                currentMode = ViewMode::NetworkMap;
            }
            if (ImGui::RadioButton("Timeline", currentMode == ViewMode::Timeline)) {
                currentMode = ViewMode::Timeline;
            }
            if (ImGui::RadioButton("Traffic Chart", currentMode == ViewMode::TrafficChart)) {
                currentMode = ViewMode::TrafficChart;
            }
        }
        ImGui::End();
    }

    void renderNetworkMap() {
        if (ImGui::Begin("Network Map")) {
            ImGui::Text("Network Map View");
            // Add logic to render network map here
        }
        ImGui::End();
    }

    void renderTimeline() {
        if (ImGui::Begin("Timeline")) {
            ImGui::Text("Timeline View");
            // Add logic to render timeline here
        }
        ImGui::End();
    }

    void renderTrafficChart() {
        if (ImGui::Begin("Traffic Chart")) {
            ImGui::Text("Traffic Chart View");
            // Add logic to render traffic chart here
        }
        ImGui::End();
    }
};
