#include "Pch.h"
#include "DirectionalLight.h"
#include "Utils/SharedConstants/BuffersConstants.h"
#include "Utils/SharedConstants/ShadowConstants.h"
#include "imgui.h"

using namespace SharedConstants;
using namespace DirectX;

DirectionalLight::DirectionalLight()
    : m_direction(BuffersConstants::LIGHT_DIR), m_diffuse(1, 1, 1, 1), m_ambient(1, 1, 1, 1){
    m_lookAt = { 0.0f, 0.0f, 0.0f };
    m_viewMatrix = XMMatrixIdentity();
    m_projectionMatrix = XMMatrixIdentity();
    m_rotationSpeed = 0.5f;
} // DirectionalLight

DirectionalLight::~DirectionalLight() {
} // ~DirectionalLight

bool DirectionalLight::Init(ID3D11Device* device, HWND  hwnd) {
    m_direction = BuffersConstants::LIGHT_DIR;
    m_diffuse = BuffersConstants::LIGHT_DIFFUSE;
    m_ambient = BuffersConstants::LIGHT_AMBIENT;
    Update();
    return true;
} // Init

void DirectionalLight::Update() {
    XMVECTOR dir = XMLoadFloat3(&m_direction);
    XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    dir = XMVector3Normalize(dir);

    float distance = 100.0f;
    XMVECTOR lookAt = XMLoadFloat3(&m_lookAt);
    XMVECTOR lightPos = lookAt - (dir * distance);

    if (abs(XMVectorGetY(dir)) > 0.999f)
    {
        upVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    }
    m_viewMatrix = XMMatrixLookAtLH(lightPos, lookAt, upVector);

    m_projectionMatrix = XMMatrixOrthographicLH(ShadowConstants::VIEW_WIDTH, ShadowConstants::VIEW_HEIGHT, ShadowConstants::NEAR_Z, ShadowConstants::FAR_Z);
} // Update

void DirectionalLight::Rotate(float deltaTime) {
    float angle = m_rotationSpeed * deltaTime;
    XMMATRIX rotationMatrix = XMMatrixRotationY(-angle);
    XMVECTOR dir = XMLoadFloat3(&m_direction);

    dir = XMVector3TransformNormal(dir, rotationMatrix);
    dir = XMVector3Normalize(dir);

    XMStoreFloat3(&m_direction, dir);
} // Rotate

void DirectionalLight::OnGui() {
    if (ImGui::CollapsingHeader("LIGHT SETTINGS", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();

        if (ImGui::SliderFloat3("Direction", &m_direction.x, -1.0f, 1.0f)) {
            XMVECTOR dir = XMLoadFloat3(&m_direction);

            // 0 벡터가 아닐 경우에만 정규화 진행
            float length = XMVector3Length(dir).m128_f32[0];
            if (length > 0.0f) {
                XMStoreFloat3(&m_direction, XMVector3Normalize(dir));
            }
            Update(); // 행렬 갱신
        }

        ImGui::ColorEdit4("Diffuse Color", &m_diffuse.x);

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::DragFloat3("Look At", &m_lookAt.x, 0.1f)) {
            Update();
        }

        ImGui::Unindent();
    }
} // OnGui

void     DirectionalLight::SetLookAt(XMFLOAT3 lookAt) { m_lookAt = XMFLOAT3(lookAt); return; }
void     DirectionalLight::SetLookAt(float x, float y, float z) { m_lookAt = XMFLOAT3(x, y, z); return; }

XMFLOAT3 DirectionalLight::GetPosition() const {
    XMVECTOR dir = XMLoadFloat3(&m_direction);
    XMVECTOR lookAt = XMLoadFloat3(&m_lookAt);

    float distance = 50.0f;
    XMFLOAT3 pos;
    XMStoreFloat3(&pos, lookAt - (dir * distance));
    return pos;
} // GetPosition

XMFLOAT3 DirectionalLight::GetDirection() const { return m_direction; }
XMFLOAT4 DirectionalLight::GetDiffuse() const { return m_diffuse; }
XMFLOAT4 DirectionalLight::GetAmbient() const { return m_ambient; }
XMFLOAT3 DirectionalLight::GetLookAt() const { return m_lookAt; }
XMMATRIX DirectionalLight::GetViewMatrix() const { return m_viewMatrix; }
XMMATRIX DirectionalLight::GetProjection() const { return m_projectionMatrix; }
