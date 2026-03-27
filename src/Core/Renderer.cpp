#include "Pch.h"
#include "Renderer.h"
#include "RendererState.h"
// Objects
#include "Objects/Stone.h"
#include "Objects/SkyBox.h"
#include "Objects/DirectionalLight.h"
#include "Objects/Ground.h"
// D3D11
#include "D3D11/D3D11Manager.h"
#include "D3D11/D3D11State.h"
#include "D3D11/D3D11CoreResources.h"
#include "D3D11/RenderTexture.h"
// Camera
#include "Camera/Camera.h"
// Resources
#include "Resources/TextureManager.h"
#include "Resources/ConstantBufferType.h"
#include "Resources/DepthRecorder.h"
// Utils
#include "ImGui/ImGuiManager.h"
#include "ImGui/ImGuiDrawer.h"
#include "ImGui/CameraWidget.h"
#include "ImGui/AssimpModelWidget.h"
#include "ImGui/FunctionWidget.h"
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/CameraConstants.h"
#include "SharedConstants/BuffersConstants.h"
#include "SharedConstants/ShadowConstants.h"

using namespace DirectX;
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
    m_Ground = std::make_unique<Ground>();
    m_DirectionalLight = std::make_unique<DirectionalLight>();
    m_DepthRecorder = std::make_unique<DepthRecorder>();
    m_shadowMapTexture = std::make_unique<RenderTexture>();
    m_TextureMgr = std::make_shared<TextureManager>();
    m_nullRTV = nullptr;
    m_nullSRV = nullptr;
    m_blendFactor[0] = 0.0f;
    m_blendFactor[1] = 0.0f;
    m_blendFactor[2] = 0.0f;
    m_blendFactor[3] = 0.0f;
    m_renderingTime = 0.0f;
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
    auto linerWrapSampler = m_D3D11Mgr->GetStates()->GetLinearWrapSamplerState();
    auto linerCampSampler = m_D3D11Mgr->GetStates()->GetLinearClampSamplerState();

    if (!m_DirectionalLight->Init(device, hwnd)) {
        return false;
    }

    if (!m_Camera->Init(CameraConstants::DEFAULT_FOV, RendererState::aspectRatio,
                        RendererState::ScreenNear, RendererState::ScreenDepth)) {
        return false;
    }

    if (!m_TextureMgr->Init(device, context, hwnd)) {
        return false;
    }

    Stone::InitParams stoneInitParams;
    stoneInitParams.device = device;
    stoneInitParams.context = context;
    stoneInitParams.hwnd = hwnd;
    stoneInitParams.textMgr = m_TextureMgr;
    stoneInitParams.path = STONE;

    if (!m_Stone->Init(stoneInitParams)) {
        return false;
    }

    SkyBox::InitParams skyInitParams;
    skyInitParams.device = device;
    skyInitParams.context = context;
    skyInitParams.hwnd = hwnd;
    skyInitParams.noiseTexture = m_TextureMgr->GetVolumeTexture(PathConstants::KEY_CLOUD_VOL);
    skyInitParams.sampler = linerWrapSampler;
    skyInitParams.depth = m_D3D11Mgr->GetDepthSRV();

    if (!m_SkyBox->Init(skyInitParams)) {
        return false;
    }

    Ground::InitParams groundInitParams;
    groundInitParams.device = device;
    groundInitParams.hwnd = hwnd;

    if (!m_Ground->Init(groundInitParams)) {
        return false;
    }

    if (!m_shadowMapTexture->Init(device, 
        ShadowConstants::SHADOWMAP_WIDTH, ShadowConstants::SHADOWMAP_HEIGHT, RenderTexture::RenderTextureType::Depth)) {
        return false;
    }

    DepthRecorder::InitParams depthParams;
    depthParams.device = device;
    depthParams.hwnd = hwnd;
    if (!m_DepthRecorder->Init(depthParams)) {
        return false;
    }

    //if (!m_VolumeSlicer->Init(device, hwnd, linerWrapSampler)) {
    //    return false;
    //}

    m_ImGuiMgr = std::move(imgui);
    if (m_ImGuiMgr && !m_ImGuiMgr->Init(hwnd, device, context)) {
        return false;
    }

    InitWidgets();
    return true;
} // Init

void Renderer::Shutdown() {
    if (m_Stone)            m_Stone.reset();
    if (m_Ground)           m_Ground.reset();
    if (m_SkyBox)           m_SkyBox.reset();
    if (m_DirectionalLight) m_DirectionalLight.reset();
    if (m_Camera)           m_Camera.reset();

    if (m_DepthRecorder)    m_DepthRecorder.reset();
    if (m_shadowMapTexture) m_shadowMapTexture.reset();
    if (m_TextureMgr)       m_TextureMgr.reset();

    if (m_ImGuiMgr)         m_ImGuiMgr.reset();
    if (m_D3D11Mgr)         m_D3D11Mgr.reset();
} // Shutdown

bool Renderer::Frame(float deltaTime) {
    m_renderingTime += deltaTime;
    m_DirectionalLight->Rotate(deltaTime);
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
    auto context = m_D3D11Mgr->GetDeviceContext();
    auto states  = m_D3D11Mgr->GetStates();

    m_Camera->Update();
    m_DirectionalLight->Update();

    DepthPass(context, states);

    MainPass(context, states);
    return true;
} // Render

void Renderer::MainPass(ID3D11DeviceContext* context, D3D11State* states) {
    m_D3D11Mgr->RestoreViewport();
    m_D3D11Mgr->BeginScene(0.15f, 0.15f, 0.15f, 1.0f);

    DrawSkyBox(context, states);
    DrawGround(context, states);
    DrawStone(context, states);

    if (m_ImGuiMgr) {
        m_ImGuiMgr->Frame();
    }

    //DebugVolume(context);
    m_D3D11Mgr->EndScene(RendererState::VsyncEnable);
} // MainPass

void Renderer::DepthPass(ID3D11DeviceContext* context, D3D11State* states) {
    context->OMSetRenderTargets(1, &m_nullRTV, m_shadowMapTexture->GetDSV());
    m_shadowMapTexture->ClearDepth(context);

    D3D11_VIEWPORT shadowViewport = {};
    shadowViewport.Width = static_cast<float>(m_shadowMapTexture->GetWidth());
    shadowViewport.Height = static_cast<float>(m_shadowMapTexture->GetHeight());
    shadowViewport.MinDepth = 0.0f;
    shadowViewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &shadowViewport);

    DepthRecorder::RenderParams depthParams;
    depthParams.viewMatrix = m_DirectionalLight->GetViewMatrix();
    depthParams.projectionMatrix = m_DirectionalLight->GetProjection();

    if (m_Stone) {
        depthParams.worldMatrix = m_Stone->GetWorldMatrix();
        m_DepthRecorder->Render(context, depthParams);
        m_Stone->DrawIndexed(context);
    }

    context->PSSetShaderResources(10, 1, &m_nullSRV);
} // DepthPass

void Renderer::DrawStone(ID3D11DeviceContext* context, D3D11State* states) {
    context->RSSetState(states->GetCullBackState());
    context->OMSetDepthStencilState(states->GetDepthState(), 1);
    context->OMSetBlendState(states->GetBlendState(), m_blendFactor, 0xffffffff);

    if (!m_Stone) return;

    m_Stone->SetSampler(states->GetLinearWrapSamplerState());

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
    context->OMSetBlendState(states->GetBlendState(), m_blendFactor, 0xffffffff);

    SkyBox::RenderParams skyParams;
    skyParams.view = m_Camera->GetViewMatrix();
    skyParams.projection = m_Camera->GetProjectionMatrix();
    skyParams.cameraPosition = m_Camera->GetPosition();
    skyParams.lightDir = m_DirectionalLight->GetDirection();
    skyParams.time = m_renderingTime;

    m_SkyBox->Render(context, skyParams);
    context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
} // DrawSkyBox

void Renderer::DrawGround(ID3D11DeviceContext* context, D3D11State* states) {
    if (!m_Ground) return;

    context->RSSetState(states->GetCullBackState());
    context->OMSetDepthStencilState(states->GetDepthState(), 1);
    context->OMSetBlendState(states->GetBlendState(), m_blendFactor, 0xffffffff);

    Ground::RenderParams groundParams;
    groundParams.view = m_Camera->GetViewMatrix();
    groundParams.projection = m_Camera->GetProjectionMatrix();
    groundParams.cameraPosition = m_Camera->GetPosition();
    groundParams.lightDir = m_DirectionalLight->GetDirection();
    groundParams.lightDiffuse = m_DirectionalLight->GetDiffuse();
    groundParams.time = m_renderingTime;
    groundParams.lightView = m_DirectionalLight->GetViewMatrix();
    groundParams.lightProjection = m_DirectionalLight->GetProjection();

    m_Ground->SetShadowMap(m_shadowMapTexture->GetSRV());
    m_Ground->SetShadowSampler(m_D3D11Mgr->GetStates()->GetShadowSamplerState());
    m_Ground->Render(context, groundParams);
} // DrawGround

//void Renderer::DebugVolume(ID3D11DeviceContext* context) {
//    auto cloudNoise = m_TextureMgr->GetVolumeTexture(KEY_CLOUD_VOL);
//    if (cloudNoise && m_VolumeSlicer) {
//        m_VolumeSlicer->Update(context, cloudNoise.get(), m_VolumeSlicer->GetDepth());
//    }
//} // DebugVolume

void Renderer::InitWidgets() {
    if (m_ImGuiMgr) {
        m_ImGuiMgr->AddWidget(std::make_unique<CameraWidget>(m_Camera.get()));

        //m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
        //    "SkyBox Atmosphere",
        //    [this]() { m_SkyBox->OnGui(); }
        //));

        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "Light Control",
            [this]() { m_DirectionalLight->OnGui(); }
        ));

        //m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
        //    "Volume Noise Debug",
        //    [this]() { m_VolumeSlicer->OnGui(); }
        //));

        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "Ground Control",
            [this]() { m_Ground->OnGui(); }
        ));
    }
} // InitWidgets