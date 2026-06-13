#include "Pch.h"
#include "ActorObject.h"
// ImGui

using namespace DirectX;

ActorObject::ActorObject() : m_RenderCount(0) {
} // ActorObject

void ActorObject::SetPosition(const XMFLOAT3& pos) {
	m_transform.SetPosition(pos);
} // SetPosition

void ActorObject::SetPosition(float x, float y, float z) {
	m_transform.SetPosition(x, y, z);
} // SetPosition

void ActorObject::SetRotation(const XMFLOAT3& rot) {
	m_transform.SetRotation(rot);
} // SetRotation

void ActorObject::SetRotation(float x, float y, float z) {
	m_transform.SetRotation(x, y, z);
} // SetRotation

void ActorObject::SetScale(const XMFLOAT3& scale) {
	m_transform.SetScale(scale);
} // SetScale

void ActorObject::SetScale(float x, float y, float z) {
	m_transform.SetScale(x, y, z);
} // SetScale

void ActorObject::Translate(const XMFLOAT3& offset) {
	m_transform.Translate(offset);
} // Translate

void ActorObject::Translate(float x, float y, float z) {
	m_transform.Translate(x, y, z);
} // Translate

void ActorObject::Rotate(const XMFLOAT3& rot) {
	m_transform.Rotate(rot);
} // Rotate

void ActorObject::Rotate(float x, float y, float z) {
	m_transform.Rotate(x, y, z);
} // Rotate

XMFLOAT3 ActorObject::GetPosition() const {
	return m_transform.GetPosition();
} // GetPosition

XMMATRIX ActorObject::GetWorldMatrix() {
	return m_transform.GetWorldMatrix();
} // GetWorldMatrix

unsigned int ActorObject::GetRenderCount() const {
	return m_RenderCount;
} // GetRenderCount

void ActorObject::DrawTransformGui() {
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.5f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.6f, 1.0f));

    if (ImGui::CollapsingHeader("TRANSFORM", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::Spacing();

        DirectX::XMFLOAT3 pos = m_transform.GetPosition();
        DirectX::XMFLOAT3 rot = m_transform.GetRotation();
        DirectX::XMFLOAT3 scale = m_transform.GetScale();

        bool isChanged = false;

        if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
            m_transform.SetPosition(pos);
            isChanged = true;
        }
        if (ImGui::DragFloat3("Rotation", &rot.x, 1.0f)) {
            m_transform.SetRotation(rot);
            isChanged = true;
        }
        if (ImGui::DragFloat3("Scale", &scale.x, 0.05f, 0.001f, 100.0f)) {
            m_transform.SetScale(scale);
            isChanged = true;
        }

        ImGui::Unindent();
        ImGui::Spacing();
    }
    ImGui::PopStyleColor(3);
} // DrawTransformGui