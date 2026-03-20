#include "Pch.h"
#include "DirectionalLight.h"
#include "Utils/SharedConstants/BuffersConstants.h"
#include "imgui.h"

using namespace SharedConstants;
using namespace DirectX;

DirectionalLight::DirectionalLight()
    : m_direction(BuffersConstants::LIGHT_DIR), m_diffuse(1, 1, 1, 1) {
} // DirectionalLight

DirectionalLight::~DirectionalLight() {
} // ~DirectionalLight

void DirectionalLight::Init() {
    m_direction = BuffersConstants::LIGHT_DIR;
    m_diffuse = BuffersConstants::LIGHT_DIFFUSE;
} // Init


void DirectionalLight::OnGui() {
    if (ImGui::CollapsingHeader("LIGHT SETTINGS", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::Spacing();

        if (ImGui::DragFloat3("Direction", &m_direction.x, 0.01f, -1.0f, 1.0f)) {
            XMVECTOR dir = XMLoadFloat3(&m_direction);
            if (XMVector3Length(dir).m128_f32[0] > 0.0f) {
                XMStoreFloat3(&m_direction, XMVector3Normalize(dir));
            }
        }

        ImGui::ColorEdit4("Diffuse Color", &m_diffuse.x);

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::Text("Normalized Dir: %.2f, %.2f, %.2f",
            m_direction.x, m_direction.y, m_direction.z);

        ImGui::Unindent();
    }
} // OnGui

XMFLOAT3   DirectionalLight::GetDirection() const { return m_direction; }
XMFLOAT4   DirectionalLight::GetDiffuse()   const { return m_diffuse; }
