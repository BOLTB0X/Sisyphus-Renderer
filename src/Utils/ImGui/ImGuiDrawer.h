#pragma once
#include "imgui.h"

namespace ImGuiDrawer {

    inline void DrawPerformance(int fps, long cpu, float frameTime) {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::Begin("PERFORMANCE", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);

        ImGui::Text("FPS: %d", fps);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "(%.2f ms)", frameTime);

        static float cpu_history[60] = {};
        for (int i = 0; i < 59; i++) {
            cpu_history[i] = cpu_history[i + 1];
        }
        cpu_history[59] = (float)cpu;

        ImGui::PlotLines("CPU", cpu_history, IM_ARRAYSIZE(cpu_history), 0, nullptr, 0.0f, 100.0f, ImVec2(0, 50));
        ImGui::Text("CPU Usage: %d%%", cpu);

        ImGui::End();
    } // DrawPerformance
} // ImGuiDrawer