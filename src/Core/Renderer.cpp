#include "Pch.h"
#include "Renderer.h"
#include "RendererState.h"
// Objects
#include "Objects/Stone.h"
#include "Objects/SkyBox.h"
#include "Objects/DirectionalLight.h"
// D3D11
#include "D3D11/D3D11Manager.h"
#include "D3D11/D3D11State.h"
#include "D3D11/D3D11CoreResources.h"
// Camera
#include "Camera/Camera.h"
// Resources
#include "Resources/TextureManager.h"
#include "Resources/ConstantBufferType.h"
// Shaders
// Utils
#include "ImGui/ImGuiManager.h"
#include "ImGui/ImGuiDrawer.h"
#include "ImGui/CameraWidget.h"
#include "ImGui/AssimpModelWidget.h"
#include "ImGui/FunctionWidget.h"
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/CameraConstants.h"
#include "SharedConstants/BuffersConstants.h"
#include "Helpers/DebugHelper.h"

using namespace SharedConstants;
using namespace PathConstants;
using namespace ConstantBuffer;
using namespace BuffersConstants;
using namespace DebugHelper;

Renderer::Renderer() {
    m_D3D11Mgr = std::make_unique<D3D11Manager>();
    m_Stone = std::make_unique<Stone>();
    m_Camera = std::make_unique<Camera>();
	m_SkyBox = std::make_unique<SkyBox>();
    m_DirectionalLight = std::make_unique<DirectionalLight>();
    m_TextureMgr = std::make_shared<TextureManager>();
} // Renderer

Renderer::Renderer(const Renderer& other) {
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
    m_DirectionalLight->Init();

    if (!m_Camera->Init(CameraConstants::DEFAULT_FOV, RendererState::aspectRatio,
                        RendererState::ScreenNear, RendererState::ScreenDepth)) {
        return false;
    }

    if (!m_TextureMgr->Init(device, context, hwnd)) {
        return false;
    }

    if (!m_Stone->Init(device, context, hwnd, m_TextureMgr, STONE)) {
        return false;
    }

    auto wSampler = m_D3D11Mgr->GetStates()->GetLinearWrapSamplerState();
    if (!m_SkyBox->Init(device, context, hwnd, wSampler, m_TextureMgr.get())) {
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
    if (m_D3D11Mgr) {
        m_D3D11Mgr.reset();
    }
} // Shutdown

bool Renderer::Frame(float deltaTime) {
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
    DrawSkyBox(context, states);

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

        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "SkyBox Atmosphere",
            [this]() { m_SkyBox->OnGui(); }
        ));

        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "Light Control",
            [this]() { m_DirectionalLight->OnGui(); }
        ));
    }
} // InitWidgets

void Renderer::DrawStone(ID3D11DeviceContext* context, D3D11State* states) {
    context->RSSetState(states->GetCullBackState());
    context->OMSetDepthStencilState(states->GetDepthState(), 1);
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context->OMSetBlendState(states->GetBlendState(), blendFactor, 0xffffffff);

    if (!m_Stone) return;

    m_Stone->SetSampler(states->GetLinearWrapSamplerState());

    // 렌더링 파라미터 구성
    Stone::RenderParams stoneParams;
    stoneParams.world = m_Stone->GetWorldMatrix();
    stoneParams.view = m_Camera->GetViewMatrix();
    stoneParams.projection = m_Camera->GetProjectionMatrix();
    stoneParams.camPos = m_Camera->GetPosition();
    stoneParams.diffuse = m_DirectionalLight->GetDiffuse();
    stoneParams.lightDir = m_DirectionalLight->GetDirection();

    m_Stone->Render(context, stoneParams);
} // DrawStone

void Renderer::DrawSkyBox(ID3D11DeviceContext* context, D3D11State* states) {
    if (!m_Camera || !m_SkyBox) return;

    ID3D11RenderTargetView* rtv = m_D3D11Mgr->GetRTV();
    context->OMSetRenderTargets(1, &rtv, nullptr);

    context->RSSetState(states->GetCullNone());
    context->OMSetDepthStencilState(states->GetDepthNone(), 0);

    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context->OMSetBlendState(states->GetBlendState(), blendFactor, 0xffffffff);

    SkyBox::RenderParams skyParams;
    skyParams.view = m_Camera->GetViewMatrix();
    skyParams.projection = m_Camera->GetProjectionMatrix();
    skyParams.cameraPosition = m_Camera->GetPosition();
    skyParams.lightDir = m_DirectionalLight->GetDirection();

    ID3D11ShaderResourceView* depthSRV = m_D3D11Mgr->GetDepthSRV();
    context->PSSetShaderResources(1, 1, &depthSRV);
    m_SkyBox->Render(context, skyParams);
    context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
} // DrawSkyBox
