#pragma once
#include "imgui.h"
// Graphics
#include "Camera/Camera.h"
#include "Resources/AssimpModel.h"
#include "Objects/Atmosphere.h"

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

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

        if (ImGui::Button("Reset to Default", ImVec2(-1, 0))) {
            camera->Reset();
        }
        ImGui::PopStyleColor(3);
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

namespace ImGuiDrawer {
    inline void DrawAssimpModel(AssimpModel* model) {
        if (!model) return;

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.5f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));

        // 모델 정보를 담은 메인 헤더
        if (ImGui::CollapsingHeader("MODEL INSPECTOR", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PopStyleColor(3);

            ImGui::Indent();
            ImGui::Spacing();

            ImGui::TextDisabled("Resource Info");
            ImGui::Text("Total Meshes: "); ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%d", model->GetMeshCount());

            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "PBR MATERIALS STATUS");
            ImGui::Spacing();

            std::vector<AssimpModel::MaterialInfo> materials = model->GetMaterialInfos();

            for (size_t i = 0; i < materials.size(); ++i) {
                const auto& mat = materials[i];

                // 각 머테리얼별 트리 노드
                if (ImGui::TreeNode((void*)(intptr_t)i, "Material [%d]: %s", (int)i, mat.name.c_str())) {

                    ImGui::BeginGroup();

                    auto ShowStatus = [](const char* type, bool isLoaded) {
                        ImGui::Text("%-10s:", type);
                        ImGui::SameLine();
                        if (isLoaded) {
                            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), " [ LOADED ]");
                        }
                        else {
                            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), " [ MISSING ]");
                        }
                        };

                    ShowStatus("Albedo", mat.hasAlbedo);
                    ShowStatus("Normal", mat.hasNormal);
                    ShowStatus("Metallic", mat.hasMetallic);
                    ShowStatus("Roughness", mat.hasRoughness);
                    ShowStatus("AO", mat.hasAO);

                    ImGui::EndGroup();
                    ImGui::Spacing();
                    ImGui::TreePop();
                }
            }

            ImGui::Unindent();
        }
        else {
            ImGui::PopStyleColor(3);
        }
    } // DrawAssimpModel

} // ImGuiDrawer - Model

namespace ImGuiDrawer {

    inline bool DrawAtmosphere(Atmosphere::AtmosphereBuffer* buffer) {
        if (!buffer) return false;
        bool changed = false;

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.3f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.4f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.3f, 0.4f, 1.0f));

        if (ImGui::CollapsingHeader("ATMOSPHERE SETTINGS", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PopStyleColor(3);
            ImGui::Indent();
            ImGui::Spacing();

            // 색상 설정
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "[ Colors ]");
            changed |= ImGui::ColorEdit4("Zenith", &buffer->zenithColor.x);
            changed |= ImGui::ColorEdit4("Horizon", &buffer->horizonColor.x);
            changed |= ImGui::ColorEdit3("Ground", &buffer->groundColor.x);
            ImGui::Separator();

            // 행성 및 대기권 크기
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "[ Geometry ]");
            changed |= ImGui::DragFloat("Planet Radius", &buffer->planetRadius, 1000.0f, 1000.0f, 1e7f, "%.0f");
            changed |= ImGui::DragFloat("Atmo Radius", &buffer->atmoRadius, 1000.0f, 1000.0f, 1e7f, "%.0f");
            changed |= ImGui::DragFloat3("Planet Center", &buffer->planetCenter.x, 100.0f, -1e7f, 1e7f, "%.0f");
            ImGui::Separator();

            // 산란 계수
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "[ Scattering Coefficients ]");
            changed |= ImGui::DragFloat3("Rayleigh Beta", &buffer->rayleighBeta.x, 0.000001f, 0.0f, 0.1f, "%.6f");
            changed |= ImGui::DragFloat("Mie Beta", &buffer->mieBeta, 0.000001f, 0.0f, 0.1f, "%.6f");
            changed |= ImGui::DragFloat3("Absorption Beta", &buffer->absorptionBeta.x, 0.000001f, 0.0f, 0.1f, "%.6f");
            changed |= ImGui::DragFloat("Ambient Beta", &buffer->ambientBeta, 0.00001f, 0.0f, 0.1f, "%.5f");
            ImGui::Separator();

            // 고도 분포 및 위상 함수
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "[ Heights & Phase ]");
            changed |= ImGui::DragFloat("Rayleigh Height", &buffer->rayleighHeight, 100.0f, 100.0f, 50000.0f, "%.0f");
            changed |= ImGui::DragFloat("Mie Height", &buffer->mieHeight, 100.0f, 100.0f, 50000.0f, "%.0f");
            changed |= ImGui::DragFloat("Absorb Height", &buffer->absorptionHeight, 100.0f, 100.0f, 50000.0f, "%.0f");
            changed |= ImGui::DragFloat("Absorb Falloff", &buffer->absorptionFalloff, 100.0f, 100.0f, 50000.0f, "%.0f");
            changed |= ImGui::SliderFloat("Mie G (Phase)", &buffer->g, -0.99f, 0.99f);
            changed |= ImGui::DragFloat("Intensity", &buffer->intensity, 0.1f, 0.0f, 100.0f, "%.2f");
            ImGui::Separator();

            // 레이마칭 품질
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "[ Raymarching Steps ]");
            changed |= ImGui::SliderInt("Primary Steps", &buffer->primarySteps, 4, 128);
            changed |= ImGui::SliderInt("Light Steps", &buffer->lightSteps, 1, 32);
            changed |= ImGui::SliderInt("Ground Pri-Steps", &buffer->groundPrimarySteps, 4, 128);
            changed |= ImGui::SliderInt("Ground Lgt-Steps", &buffer->groundLightSteps, 1, 32);

            ImGui::Unindent();
        }
        else {
            ImGui::PopStyleColor(3);
        }

        return changed; // 값이 하나라도 변경되면 true 반환
    } // DrawAtmosphere
} // ImGuiDrawer