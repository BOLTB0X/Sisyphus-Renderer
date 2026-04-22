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
#include "Data/ShadowMap.h"
#include "Data/CloudMap.h"
// Resources
#include "Resources/ConstantBufferType.h"
#include "Resources/VolumeTexture.h"
#include "Resources/Texture.h"
// Post
#include "Post/CloudComposite.h"
#include "Post/TAA.h"
#include "Post/Bloom.h"
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
    m_ShadowMap = std::make_unique<ShadowMap>();
	m_CloudMapLUT = std::make_unique<CloudMap>();
	m_AtmosphereLUT = std::make_unique<Atmosphere>();
	m_Composite = std::make_unique<CloudComposite>();
    m_Bloom = std::make_unique<Bloom>();
    m_TAA = std::make_unique<TAA>();
    m_TextureMgr = std::make_shared<TextureManager>();
    m_sceneRT = std::make_unique<RenderTexture>();
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
	stoneInitParams.linerSampler = linerWrapSampler;

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
	cloudInitParams.cloudMapLUTSRV = m_CloudMapLUT->GetSRV();
	cloudInitParams.worleyNoiseSRV = m_TextureMgr->GetVolumeTexture(PathConstants::KEY_WORLEY_NOISE)->GetSRV();
	cloudInitParams.blueNoiseSRV = m_TextureMgr->GetTexture(device, context, PathConstants::BLUE_NOISE)->GetSRV();
	cloudInitParams.wrapSampler = linerWrapSampler;

    if (!m_VolumetricCloud->Init(cloudInitParams)) {
        return false;
    }

    ShadowMap::InitParams shadowParams;
    shadowParams.device = device;
    shadowParams.hwnd = hwnd;
    if (!m_ShadowMap->Init(shadowParams)) {
        return false;
    }

    if (!m_sceneRT->Init(m_D3D11Mgr->GetDevice(),
        RendererState::ScreenWidth, RendererState::ScreenHeight,
        RenderTexture::RenderTextureType::Normal, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        return false;
    }
    
    CloudComposite::InitParams compositeParams;
    compositeParams.device = device;
    compositeParams.hwnd = hwnd;
    compositeParams.vPath = PathConstants::POST_VS;
    compositeParams.pPath = PathConstants::CLOUD_COMPOSITE_PS;
    compositeParams.ScreenWidth = RendererState::ScreenWidth;
    compositeParams.ScreenHeight = RendererState::ScreenHeight;
    if (!m_Composite->Init(compositeParams)) {
        return false;
    }

    Bloom::InitParams bloomParams;
    bloomParams.device = device;
    bloomParams.hwnd = hwnd;
    bloomParams.vPath = PathConstants::POST_VS;
    bloomParams.pPath = PathConstants::BLOOM_PS;
    bloomParams.ScreenWidth = RendererState::ScreenWidth;
    bloomParams.ScreenHeight = RendererState::ScreenHeight;
    if (!m_Bloom->Init(bloomParams)) {
        return false;
    }

    TAA::InitParams taaParams;
    taaParams.device = device;
    taaParams.hwnd = hwnd;
    taaParams.vPath = PathConstants::POST_VS;
    taaParams.pPath = PathConstants::TAA_PS;
    taaParams.ScreenWidth = RendererState::ScreenWidth;
    taaParams.ScreenHeight = RendererState::ScreenHeight;

    if (!m_TAA->Init(taaParams)) {
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
    if (m_ShadowMap) {
        m_ShadowMap.reset();
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

    ShadowPass(context, states);
    MainPass(context, states);
    PostProcessing(context, states);

    m_ImGuiMgr->Frame();
    m_D3D11Mgr->EndScene(RendererState::VsyncEnable);
    return true;
} // Render

void Renderer::ShadowPass(ID3D11DeviceContext* context, D3D11State* states) {
    context->OMSetRenderTargets(1, &m_nullRTV, m_ShadowMap->GetDSV());
    m_ShadowMap->ClearShadowDepth(context);

    const auto& viewport = m_ShadowMap->GetViewport();
    context->RSSetViewports(1, &viewport);

    ShadowMap::RenderParams renderParams;
    renderParams.viewMatrix = m_DirectionalLight->GetViewMatrix();
    renderParams.projectionMatrix = m_DirectionalLight->GetProjection();

    if (m_Stone) {
        renderParams.worldMatrix = m_Stone->GetWorldMatrix();
        m_ShadowMap->Render(context, renderParams);
        m_Stone->DrawIndexed(context);
    }

    context->PSSetShaderResources(10, 1, &m_nullSRV);
} // ShadowPass

void Renderer::MainPass(ID3D11DeviceContext* context, D3D11State* states) {
    static bool isFirst = true;
    m_D3D11Mgr->RestoreViewport();

    float clearColor[4] = { 0.15f, 0.15f, 0.15f, 1.0f };
    ID3D11RenderTargetView* sceneRTV = m_sceneRT->GetRTV();
    context->OMSetRenderTargets(1, &sceneRTV, m_D3D11Mgr->GetDepthRT()->GetDSV());
    context->ClearRenderTargetView(sceneRTV, clearColor);
    m_D3D11Mgr->GetDepthRT()->ClearDepth(context, 1.0f, 0);

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

    DrawGround(context, states);
    DrawStone(context, states);
    DrawSkyBox(context, states);
	ComputeShaderData(context, states);
} // MainPass

void Renderer::PostProcessing(ID3D11DeviceContext* context, D3D11State* states) {
    static bool isFirst = true;

    // composite
    {
        context->PSSetShaderResources(0, 1, &m_nullSRV);
        context->PSSetShaderResources(1, 1, &m_nullSRV);
        ID3D11RenderTargetView* compositeRTV = m_Composite->GetRTV();
        context->OMSetRenderTargets(1, &compositeRTV, nullptr);
        m_Composite->ClearRT(context);

        CloudComposite::RenderParams renderParam;
        renderParam.sceneSRV = m_sceneRT->GetSRV();
        renderParam.cloudSRV = m_VolumetricCloud->GetCloudSRV();
        renderParam.linerSampler = states->GetLinearWrapSamplerState();
        m_Composite->Render(context, renderParam);
    }

    // Bloom
    {
        context->PSSetShaderResources(0, 1, &m_nullSRV);

        ID3D11RenderTargetView* bloomRTV = m_Bloom->GetRTV();
        context->OMSetRenderTargets(1, &bloomRTV, nullptr);
        m_Bloom->ClearRT(context);

        Bloom::RenderParams bloomParam;
        bloomParam.inputSRV = m_Composite->GetSRV();
        bloomParam.linerSampler = states->GetLinearWrapSamplerState();
        m_Bloom->Render(context, bloomParam);
    }

    //// TAA
    {
        context->PSSetShaderResources(0, 1, &m_nullSRV);
        context->PSSetShaderResources(1, 1, &m_nullSRV);

        ID3D11RenderTargetView* backRTV = m_D3D11Mgr->GetRTV();
        context->OMSetRenderTargets(1, &backRTV, nullptr);

        TAA::RenderParams taaParam;
        taaParam.currentSRV = m_Bloom->GetSRV();
        taaParam.linerSampler = states->GetLinearWrapSamplerState();
        taaParam.blendFactor = isFirst ? 0.0f : 0.85f;
        taaParam.texelSize = XMFLOAT2(
            1.0f / RendererState::ScreenWidth,
            1.0f / RendererState::ScreenHeight);
        m_TAA->Render(context, taaParam);

        isFirst = false;
        m_TAA->CopyResource(context, m_Bloom->GetTexture());
    }
} // PostProcessing

void Renderer::DrawStone(ID3D11DeviceContext* context, D3D11State* states) {
    context->RSSetState(states->GetCullBackState());
    context->OMSetDepthStencilState(states->GetDepthState(), 1);
    context->OMSetBlendState(states->GetBlendState(), m_blendFactor, 0xffffffff);

    if (!m_Stone) return;

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
    groundParams.shadowSRV = m_ShadowMap->GetSRV();
    groundParams.shadowSampler = states->GetShadowSamplerState();
    m_Ground->Render(context, groundParams);
} // DrawGround

void Renderer::ComputeShaderData(ID3D11DeviceContext* context, D3D11State* states) {
    Atmosphere::ExecuteParams atmoExecParams;
    atmoExecParams.LightDirection = m_DirectionalLight->GetDirection();
    atmoExecParams.CameraPosition = m_Camera->GetPosition();
    atmoExecParams.time = m_renderingTime;

    m_AtmosphereLUT->Execute(context, atmoExecParams);

    VolumetricCloud::ExecuteParams cloudExecParams;
    cloudExecParams.time = m_renderingTime;
    cloudExecParams.SkyLUTSRV = m_AtmosphereLUT->GetLUT();
	cloudExecParams.depthSRV = m_D3D11Mgr->GetDepthSRV();
    m_VolumetricCloud->Execute(context, cloudExecParams);
} // ComputeShaderData

void Renderer::UpadteWidgets() {
    if (m_ImGuiMgr) {
        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "Stone Control",
            [this]() { m_Camera->OnGui(); }
		));

        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "Light Control",
            [this]() { m_DirectionalLight->OnGui(); }
        ));

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