#pragma once
#include "imgui.h"
// Graphics
#include "Camera/Camera.h"

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

    inline void DrawCamera(Camera* camera) {
        if (!camera) return;

        ImGui::SetNextWindowPos(ImVec2(10, 150), ImGuiCond_FirstUseEver);
        ImGui::Begin("CAMERA CONTROL", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        if (ImGui::Button("Reset to Default", ImVec2(-1, 0))) {
            camera->Reset();
        }
        ImGui::Separator();

        DirectX::XMFLOAT3 pos = camera->GetPosition();
        if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
            camera->SetPosition(pos);
        }

        DirectX::XMFLOAT3 rot = camera->GetRotation();
        if (ImGui::DragFloat3("Rotation", &rot.x, 0.5f, -360.0f, 360.0f)) {
            camera->SetRotation(rot);
        }

        ImGui::Separator();

        float fov = camera->GetFov();
        if (ImGui::SliderFloat("FOV", &fov, 10.0f, 120.0f, "%.1f deg")) {
            camera->SetFov(fov);
        }

        float nearP = camera->GetNear();
        float farP = camera->GetFar();
        ImGui::Text("Near: %.2f / Far: %.2f", nearP, farP);

        camera->Update();

        ImGui::End();
    } // DrawCamera
    
} // ImGuiDrawer