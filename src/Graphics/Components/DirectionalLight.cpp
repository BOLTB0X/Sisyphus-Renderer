#include "Pch.h"
#include "DirectionalLight.h"
#include "Utils/SharedConstants/BuffersConstants.h"
#include "Utils/SharedConstants/ScreenConstants.h"
#include "Utils/SharedConstants/ShadowConstants.h"
#include "imgui.h"

using namespace SharedConstants;
using namespace ScreenConstants;
using namespace DirectX;

DirectionalLight::DirectionalLight()
    : m_direction(BuffersConstants::LIGHT_DIR), m_diffuse(1, 1, 1, 1), m_ambient(1, 1, 1, 1)
    , m_sunset(0,0,0,0), m_night(0,0,0,0) {
    m_lookAt = { 0.0f, 0.0f, 0.0f };
    m_viewMatrix = XMMatrixIdentity();
    m_projectionMatrix = XMMatrixIdentity();
	m_objectViewMatrix = XMMatrixIdentity();
	m_objectProjMatrix = XMMatrixIdentity();
    m_rotationSpeed = 0.5f;
} // DirectionalLight

DirectionalLight::~DirectionalLight() {
} // ~DirectionalLight

bool DirectionalLight::Init(ID3D11Device* device, HWND  hwnd) {
    m_direction = BuffersConstants::LIGHT_DIR;
    m_diffuse = BuffersConstants::LIGHT_DIFFUSE;
    m_ambient = BuffersConstants::LIGHT_AMBIENT;
    m_sunset = BuffersConstants::SUNSET_LIGHT_COLOR;
    m_night = BuffersConstants::NIGHT_LIGHT_COLOR;

    Update();
    return true;
} // Init

void DirectionalLight::Update() {
    XMVECTOR dir = XMLoadFloat3(&m_direction);
    XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    dir = XMVector3Normalize(dir);

    XMVECTOR lookAt = XMLoadFloat3(&m_lookAt);
    XMVECTOR lightPos = lookAt - (dir * 1000.0f);

    if (abs(XMVectorGetY(dir)) > 0.999f)
        upVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    m_viewMatrix = XMMatrixLookAtLH(lightPos, lookAt, upVector);
    m_projectionMatrix = XMMatrixOrthographicLH(ShadowConstants::TERRAIN_VIEW_WIDTH,
        ShadowConstants::TERRAIN_VIEW_HEIGHT,
        ShadowConstants::NEAR_Z,
        ShadowConstants::FAR_Z
    );
} // Animate

void DirectionalLight::UpdateObjectShadow(const XMFLOAT3& objectPos) {
    XMVECTOR dir = XMLoadFloat3(&m_direction);
    dir = XMVector3Normalize(dir);

    XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    if (abs(XMVectorGetY(dir)) > 0.999f)
        upVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    XMVECTOR lookAt = XMLoadFloat3(&objectPos);
    XMVECTOR lightPos = lookAt - (dir * 100.0f);

    m_objectViewMatrix = XMMatrixLookAtLH(lightPos, lookAt, upVector);
    m_objectProjMatrix = XMMatrixOrthographicLH(ShadowConstants::OBJECT_VIEW_WIDTH,
        ShadowConstants::OBJECT_VIEW_HEIGHT,
        ShadowConstants::NEAR_Z,
        ShadowConstants::OBJECT_FAR_Z
    );
} // UpdateObjectShadow

void DirectionalLight::Rotate(float deltaTime) {
    float angle = m_rotationSpeed * deltaTime;
    XMMATRIX rotationMatrix = XMMatrixRotationY(-angle);
    XMVECTOR dir = XMLoadFloat3(&m_direction);

    dir = XMVector3TransformNormal(dir, rotationMatrix);
    dir = XMVector3Normalize(dir);

    XMStoreFloat3(&m_direction, dir);
} // Rotate

void DirectionalLight::OnGui() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

    if (ImGui::Button("Reset to Default", ImVec2(-1, 0))) {
        m_direction = BuffersConstants::LIGHT_DIR;
        m_diffuse = BuffersConstants::LIGHT_DIFFUSE;
        m_ambient = BuffersConstants::LIGHT_AMBIENT;
        m_sunset = BuffersConstants::SUNSET_LIGHT_COLOR;
        m_night = BuffersConstants::NIGHT_LIGHT_COLOR;

        Update();
    }

    ImGui::PopStyleColor(3);
    ImGui::Separator();

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

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ HDR Light Colors ]");

        ImGuiColorEditFlags hdrFlags = ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float;
        ImGui::ColorEdit4("Day (Diffuse)", &m_diffuse.x, hdrFlags);
        ImGui::ColorEdit4("Sunset", &m_sunset.x, hdrFlags);
        ImGui::ColorEdit4("Night", &m_night.x, hdrFlags);
        ImGui::ColorEdit4("Ambient", &m_ambient.x, hdrFlags);

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::DragFloat3("Look At", &m_lookAt.x, 0.1f)) {
            Update();
        }
    }
} // OnGui

void DirectionalLight::SetLookAt(XMFLOAT3 lookAt) { 
    m_lookAt = XMFLOAT3(lookAt);
    return;
} // DirectionalLight

void DirectionalLight::SetLookAt(float x, float y, float z) { 
    m_lookAt = XMFLOAT3(x, y, z);
    return;
} // SetLookAt

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
XMFLOAT4 DirectionalLight::GetSunset() const { return m_sunset; }
XMFLOAT4 DirectionalLight::GetLight() const { return m_night; }
XMFLOAT3 DirectionalLight::GetLookAt() const { return m_lookAt; }
XMMATRIX DirectionalLight::GetViewMatrix() const { return m_viewMatrix; }
XMMATRIX DirectionalLight::GetProjection() const { return m_projectionMatrix; }
XMMATRIX DirectionalLight::GetObjectViewMatrix() const { return m_objectViewMatrix; }
XMMATRIX DirectionalLight::GetObjectProjection() const { return m_objectProjMatrix; }

XMFLOAT2 DirectionalLight::GetUV(const XMMATRIX& view, const XMMATRIX& proj) const {
    XMVECTOR dir = XMLoadFloat3(&m_direction);
    XMVECTOR sunDir = XMVectorNegate(dir); // 태양을 바라보는 방향

    XMMATRIX vp = view * proj;
    XMVECTOR clip = XMVector4Transform(
        XMVectorSetW(sunDir, 0.0f), // w=0
        vp
    );

    float x = XMVectorGetX(clip);
    float y = XMVectorGetY(clip);
    float w = XMVectorGetW(clip);

    if (w <= 0.0f) return XMFLOAT2(-1.0f, -1.0f);

    float ndcX = x / w;
    float ndcY = -y / w; // Y 반전

    if (ndcX < -1.f || ndcX > 1.f || ndcY < -1.f || ndcY > 1.f)
        return XMFLOAT2(-1.0f, -1.0f);

    return XMFLOAT2(
        ndcX * 0.5f + 0.5f,
        ndcY * 0.5f + 0.5f
    );
} // GetUV