#include "Pch.h"
#include "Renderer.h"
#include "RendererState.h"
// Objects
#include "Objects/Stone.h"
#include "Objects/SkyBox.h"
#include "Objects/Ground.h"
// Components
#include "Components/DirectionalLight.h"
#include "Components/Camera.h"
#include "Components/D3D11Manager.h"
#include "Components/TextureManager.h"
// D3D11
#include "D3D11/D3D11State.h"
#include "D3D11/D3D11CoreResources.h"
// Data
#include "Data/RenderTexture.h"
#include "Data/Atmosphere.h"
#include "Data/VolumetricCloud.h"
#include "Data/DepthRecorder.h"
#include "Data/CloudMap.h"
// Resources
#include "Resources/ConstantBufferType.h"
#include "Resources/VolumeTexture.h"
#include "Resources/Texture.h"
// Post
#include "Post/Composite.h"
// Utils
#include "Helpers/ShaderHelper.h"
#include "ImGui/ImGuiManager.h"
#include "ImGui/FunctionWidget.h"
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/CameraConstants.h"
#include "SharedConstants/BuffersConstants.h"
#include "SharedConstants/ShadowConstants.h"
// define
#define FRAME_CB_SLOT 0
#define DIRL_CB_SLOT  1

using namespace DirectX;
using namespace SharedConstants;
using namespace PathConstants;
using namespace ConstantBuffer;
using namespace BuffersConstants;
using namespace DebugHelper;
using namespace ShaderHelper;

Renderer::Renderer() {
    m_D3D11Mgr = std::make_unique<D3D11Manager>();
    m_Stone = std::make_unique<Stone>();
    m_Camera = std::make_unique<Camera>();
	m_SkyBox = std::make_unique<SkyBox>();
    m_Ground = std::make_unique<Ground>();
    m_DirectionalLight = std::make_unique<DirectionalLight>();
	m_VolumetricCloud = std::make_unique<VolumetricCloud>();
    m_DepthRecorder = std::make_unique<DepthRecorder>();
	m_CloudMapLUT = std::make_unique<CloudMap>();
    m_shadowRT = std::make_unique<RenderTexture>();
	m_AtmosphereLUT = std::make_unique<Atmosphere>();
	m_Composite = std::make_unique<Composite>();
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

    if (!InitConstantBuffer<FrameBuffer>(device, m_frameBuffer.GetAddressOf())) {
        return false;
    }
    
    if (!InitConstantBuffer<DirectionalLightBuffer>(device, m_lightBuffer.GetAddressOf())) {
		return false;
    }

    if (!m_DirectionalLight->Init(device, hwnd)) {
        return false;
    }

    if (!m_Camera->Init(CameraConstants::DEFAULT_FOV, RendererState::aspectRatio,
                        RendererState::ScreenNear, RendererState::ScreenDepth)) {
        return false;
    }

    if (!m_TextureMgr->Init(device, context, hwnd)) {
		DebugHelper::DebugPrint("Failed to initialize TextureManager.");
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

    CloudMap::InitParams cloudMapParams;
    cloudMapParams.device = device;
    cloudMapParams.hwnd = hwnd;
    if (!m_CloudMapLUT->Init(cloudMapParams)) {
        return false;
    }
    else {
        m_CloudMapLUT->Generate(context);
    }

	Atmosphere::InitParams atmoParams;
	atmoParams.device = device;
	atmoParams.context = context;
	atmoParams.hwnd = hwnd;
	atmoParams.linerWrapSampler = linerWrapSampler;

    if (!m_AtmosphereLUT->Init(atmoParams)) {
        return false;
	}

    SkyBox::InitParams skyInitParams;
    skyInitParams.device = device;
    skyInitParams.context = context;
    skyInitParams.hwnd = hwnd;
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

	VolumetricCloud::InitParams cloudInitParams;
	cloudInitParams.device = device;
	cloudInitParams.hwnd = hwnd;
	cloudInitParams.CloudMapLUTSRV = m_CloudMapLUT->GetSRV();
	cloudInitParams.worleyNoiseSRV = m_TextureMgr->GetVolumeTexture(PathConstants::KEY_WORLEY_NOISE)->GetSRV();
	cloudInitParams.blueNoiseSRV = m_TextureMgr->GetTexture(device, context, PathConstants::BLUE_NOISE)->GetSRV();
	cloudInitParams.sampler = linerWrapSampler;

    if (!m_VolumetricCloud->Init(cloudInitParams)) {
        return false;
    }

    if (!m_shadowRT->Init(device, 
        ShadowConstants::SHADOWMAP_WIDTH, ShadowConstants::SHADOWMAP_HEIGHT, RenderTexture::RenderTextureType::Depth)) {
        return false;
    }

    DepthRecorder::InitParams depthParams;
    depthParams.device = device;
    depthParams.hwnd = hwnd;
    if (!m_DepthRecorder->Init(depthParams)) {
        return false;
    }
    
    Composite::InitParams compositeParams;
    compositeParams.device = device;
    compositeParams.hwnd = hwnd;
    compositeParams.vPath = PathConstants::COMPOSITE_VS;
    compositeParams.pPath = PathConstants::COMPOSITE_PS;
    if (!m_Composite->Init(compositeParams)) {
        return false;
    }

    m_ImGuiMgr = std::move(imgui);
    if (m_ImGuiMgr && !m_ImGuiMgr->Init(hwnd, device, context)) {
        return false;
    }

    UpadteWidgets();
    return true;
} // Init

void Renderer::Shutdown() {
    // [최상위 UI 레이어]
    if (m_ImGuiMgr) {
        m_ImGuiMgr.reset();
    }
    //  [렌더링 객체]
    if (m_SkyBox) {
        m_SkyBox.reset();
    }
    if (m_AtmosphereLUT) {
        m_AtmosphereLUT.reset();
	}
    if (m_Stone) {
        m_Stone.reset();
    }
    if (m_Ground) {
        m_Ground.reset();
    }
    // [리소스 생성기 및 중간 데이터]
    if (m_CloudMapLUT) {
        m_CloudMapLUT.reset();
    }
    if (m_Composite) {
        m_Composite.reset();
	}
    if (m_DepthRecorder) {
        m_DepthRecorder.reset();
    }
    if (m_shadowRT) {
        m_shadowRT.reset();
    }
    // [렌더러 핵심 컴포넌트]
    if (m_DirectionalLight) {
        m_DirectionalLight.reset();
    }
    if (m_Camera) {
        m_Camera.reset();
    }
    if (m_TextureMgr) {
        m_TextureMgr.reset();
    }
    // 그래픽스 API 코어
    if (m_D3D11Mgr) {
        m_D3D11Mgr.reset();
    }
} // Renderer

bool Renderer::Frame(float deltaTime) {
    m_renderingTime += deltaTime;
    //m_DirectionalLight->Rotate(deltaTime);
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

    FrameBuffer fData;
    fData.view = XMMatrixTranspose(m_Camera->GetViewMatrix());
    fData.projection = XMMatrixTranspose(m_Camera->GetProjectionMatrix());
    fData.viewInv = XMMatrixTranspose(XMMatrixInverse(nullptr, m_Camera->GetViewMatrix()));
    fData.projInv = XMMatrixTranspose(XMMatrixInverse(nullptr, m_Camera->GetProjectionMatrix()));
    fData.cameraPosition = m_Camera->GetPosition();
	fData.cameraFov = m_Camera->GetFov();
    fData.screenResolution = XMFLOAT2((float)RendererState::ScreenWidth, (float)RendererState::ScreenHeight);
    fData.time = m_renderingTime;

    DirectionalLightBuffer lData;
    lData.direction = m_DirectionalLight->GetDirection();
    lData.ambient = m_DirectionalLight->GetAmbient();
    lData.diffuse = m_DirectionalLight->GetDiffuse();
    lData.lightViewMatrix = XMMatrixTranspose(m_DirectionalLight->GetViewMatrix());
    lData.lightProjectionMatrix = XMMatrixTranspose(m_DirectionalLight->GetProjection());

    if (!UpdateConstantBuffer(context, m_frameBuffer.Get(), fData)) {
        DebugPrint("프레이버퍼 확인 필요");
    }
    if (!UpdateConstantBuffer(context, m_lightBuffer.Get(), lData)) {
        DebugPrint("방향광 버퍼 확인 필요");
    }

    // Slot 0: Frame, Slot 1: Light
    context->VSSetConstantBuffers(FRAME_CB_SLOT, 1, m_frameBuffer.GetAddressOf());
    context->VSSetConstantBuffers(DIRL_CB_SLOT, 1, m_lightBuffer.GetAddressOf());
    context->PSSetConstantBuffers(FRAME_CB_SLOT, 1, m_frameBuffer.GetAddressOf());
    context->PSSetConstantBuffers(DIRL_CB_SLOT, 1, m_lightBuffer.GetAddressOf());
    context->CSSetConstantBuffers(FRAME_CB_SLOT, 1, m_frameBuffer.GetAddressOf());
    context->CSSetConstantBuffers(DIRL_CB_SLOT, 1, m_lightBuffer.GetAddressOf());

	Compute(context, states);

    DrawGround(context, states);
    DrawStone(context, states);
    DrawSkyBox(context, states);

    if (m_ImGuiMgr) {
        m_ImGuiMgr->Frame();
    }

    m_D3D11Mgr->EndScene(RendererState::VsyncEnable);
} // MainPass

void Renderer::DepthPass(ID3D11DeviceContext* context, D3D11State* states) {
    context->OMSetRenderTargets(1, &m_nullRTV, m_shadowRT->GetDSV());
    m_shadowRT->ClearDepth(context);

    D3D11_VIEWPORT shadowViewport = {};
    shadowViewport.Width = static_cast<float>(m_shadowRT->GetWidth());
    shadowViewport.Height = static_cast<float>(m_shadowRT->GetHeight());
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
    m_Stone->Render(context, stoneParams);
} // DrawStone

void Renderer::DrawSkyBox(ID3D11DeviceContext* context, D3D11State* states) {
    if (!m_Camera || !m_SkyBox) return;

    context->RSSetState(states->GetCullNone());
    context->OMSetDepthStencilState(states->GetDepthLessEqual(), 1);
    context->OMSetBlendState(states->GetBlendState(), m_blendFactor, 0xffffffff);

    SkyBox::RenderParams skyParams;
	skyParams.camPos = m_Camera->GetPosition();
    skyParams.time = m_renderingTime;
	skyParams.skyLUT = m_AtmosphereLUT->GetLUT();

    m_SkyBox->Render(context, skyParams);
    context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
} // DrawSkyBox

void Renderer::DrawGround(ID3D11DeviceContext* context, D3D11State* states) {
    if (!m_Ground) return;

    context->RSSetState(states->GetCullBackState());
    context->OMSetDepthStencilState(states->GetDepthState(), 1);
    context->OMSetBlendState(states->GetBlendState(), m_blendFactor, 0xffffffff);

    Ground::RenderParams groundParams;
    groundParams.cameraPosition = m_Camera->GetPosition();
    groundParams.time = m_renderingTime;

    m_Ground->SetShadowMap(m_shadowRT->GetSRV());
    m_Ground->SetShadowSampler(m_D3D11Mgr->GetStates()->GetShadowSamplerState());
    m_Ground->Render(context, groundParams);
} // DrawGround

void Renderer::Compute(ID3D11DeviceContext* context, D3D11State* states) {
    Atmosphere::ExecuteParams atmoExecParams;
    atmoExecParams.LightDirection = m_DirectionalLight->GetDirection();
    atmoExecParams.CameraPosition = m_Camera->GetPosition();
    atmoExecParams.time = m_renderingTime;

    m_AtmosphereLUT->Execute(context, atmoExecParams);

    VolumetricCloud::ExecuteParams cloudExecParams;
    cloudExecParams.time = m_renderingTime;
    cloudExecParams.SkyLUTSRV = m_AtmosphereLUT->GetLUT();
    m_VolumetricCloud->Execute(context, cloudExecParams);
} // Compute

void Renderer::UpadteWidgets() {
    if (m_ImGuiMgr) {
        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "Stone Control",
            [this]() { m_Camera->OnGui(); }
		));

  //      m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
  //          "Light Control",
  //          [this]() { m_DirectionalLight->OnGui(); }
  //      ));

  //      m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
  //          "Ground Control",
  //          [this]() { m_Ground->OnGui(); }
  //      ));

        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "Atmosphere Control",
            [this]() { m_AtmosphereLUT->OnGui(); }
        ));


        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "Volumetric Cloud Control",
            [this]() { m_VolumetricCloud->OnGui(); }
        ));
    }
} // UpadteWidgets