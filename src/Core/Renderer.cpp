#include "Pch.h"
#include "Renderer.h"
#include "RendererState.h"
// Objects
#include "Objects/Triangle.h"
#include "Objects/Stone.h"
// D3D11
#include "D3D11/D3D11Manager.h"
#include "D3D11/D3D11State.h"
#include "D3D11/D3D11CoreResources.h"
// Camera
#include "Camera/Camera.h"
// Resources
#include "Resources/TextureManager.h"
// Utils
#include "ImGui/ImGuiManager.h"
#include "ImGui/ImGuiDrawer.h"
#include "ImGui/CameraWidget.h"
#include "ImGui/AssimpModelWidget.h"
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/CameraConstants.h"

using namespace SharedConstants;

Renderer::Renderer() {
    m_D3D11Mgr = std::make_unique<D3D11Manager>();
    m_Triangle = std::make_unique<Triangle>();
    m_Stone = std::make_unique<Stone>();
    m_Camera = std::make_unique<Camera>();
    m_TextureMgr = std::make_shared<TextureManager>();
} // Renderer

Renderer::Renderer(const Renderer& other) {
    m_D3D11Mgr = std::make_unique<D3D11Manager>();
} // Renderer

Renderer::~Renderer() {
} // ~Renderer

bool Renderer::Init(HWND hwnd, std::shared_ptr<ImGuiManager> imgui) {
    if (!m_D3D11Mgr->Init(hwnd,
                          RendererState::ScreenWidth, RendererState::ScreenHeight,
                          RendererState::FullScrren, RendererState::VsyncEnable)) {
        return false;
    }

    auto device = m_D3D11Mgr->GetDevice();
    auto context = m_D3D11Mgr->GetDeviceContext();

    if (!m_Camera->Init(CameraConstants::DEFAULT_FOV, RendererState::aspectRatio,
                        RendererState::ScreenNear, RendererState::ScreenDepth)) {
        return false;
    }

    // Stone 초기화 
    if (!m_Stone->Init(device, context, hwnd, m_TextureMgr, PathConstants::STONE)) {
        return false;
    }

    // ImGui 초기화
    m_ImGuiMgr = std::move(imgui);
    if (m_ImGuiMgr && !m_ImGuiMgr->Init(hwnd, device, context)) {
        return false;
    }

    InitWidgets();
    return true;
} // Init

void Renderer::Shutdown() {
    if (m_Stone)
        m_Stone.reset();
    if (m_Triangle)
        m_Triangle->Shutdown();
    if (m_D3D11Mgr) {
        m_D3D11Mgr.reset();
    }
} // Shutdown

bool Renderer::Frame(float deltaTime) {
    float rotationSpeed = 30.0f;
    m_Stone->Rotate(0.0f, rotationSpeed * deltaTime, 0.0f);
    return Render();
} // Frame

void Renderer::UpdateCameraRotation(float dx, float dy) {
    m_Camera->AddYaw(dx);
    m_Camera->AddPitch(dy);
} // UpdateCameraRotation

void Renderer::UpdateCameraZoom(float delta) {
    float fovDelta = -delta * 0.05f; 
    m_Camera->AddFOV(fovDelta);
} // UpdateCameraZoom

void Renderer::UpdateCameraForwardBack(float delta) {
    m_Camera->MoveForwardBack(delta);
} // UpdateCameraForwardBack

void Renderer::UpdateCameraLeftRight(float delta) {
    m_Camera->MoveLeftRight(delta);
} // UpdateCameraLeftRight

void Renderer::UpdateCameraUpDown(float delta) {
    m_Camera->MoveUpDown(delta);
} // UpdateCameraUpDown

bool Renderer::Render() {
    m_D3D11Mgr->BeginScene(0.15f, 0.15f, 0.15f, 1.0f);
    auto context = m_D3D11Mgr->GetDeviceContext();
    auto states  = m_D3D11Mgr->GetStates();

    m_Camera->Update();

    // GPU 상태 세팅
    context->RSSetState(states->GetRasterizerState());
    context->OMSetDepthStencilState(states->GetDepthStencilState(), 1);
    context->OMSetBlendState(states->GetBlendState(), nullptr, 0xffffffff);
    
	DrawStone(context, states);

    if (m_ImGuiMgr) {
        m_ImGuiMgr->Frame(); 
    }

    m_D3D11Mgr->EndScene(RendererState::VsyncEnable);

    return true;
} // Render

void Renderer::InitWidgets() {
    if (m_ImGuiMgr) {
        m_ImGuiMgr->AddWidget(std::make_unique<CameraWidget>(m_Camera.get()));
        m_ImGuiMgr->AddWidget(std::make_unique<AssimpModelWidget>(m_Stone.get()));
    }
} // InitWidgets

void Renderer::DrawStone(ID3D11DeviceContext* context, D3D11State* states) {
    if (!m_Stone) return;

    ID3D11SamplerState* sampler = states->GetLinearSamplerState();
    m_Stone->SetSampler(sampler);

    // 렌더링 파라미터 구성
    Stone::RenderParams stoneParams;
    stoneParams.world = m_Stone->GetWorldMatrix();
    stoneParams.view = m_Camera->GetViewMatrix();
    stoneParams.projection = m_Camera->GetProjectionMatrix();
    stoneParams.camPos = m_Camera->GetPosition();
    stoneParams.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
    stoneParams.lightDir = { 0.5f, -1.0f, 0.5f };

    m_Stone->Render(context, stoneParams);
} // DrawStone

void Renderer::DrawTriangle(ID3D11DeviceContext* context, D3D11State* states) {
    if (!m_Triangle) return;

    // ID3D11SamplerState* sampler = states->GetLinearSamplerState();
    // context->PSSetSamplers(0, 1, &sampler);

    m_Triangle->Render(context);
} // DrawTriangle