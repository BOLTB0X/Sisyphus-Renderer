#include "Pch.h"
#include "Renderer.h"
#include "RendererState.h"
// Objects
#include "Objects/SkyBox.h"
#include "Objects/Ground.h"
#include "Objects/Grass.h"
#include "Objects/Tree.h"
#include "Objects/MayaActor.h"
#include "Objects/SkinnedActor.h"
#include "Objects/RigidActor.h"
#include "Objects/Water.h"
// Components
#include "Components/DirectionalLight.h"
#include "Components/Camera.h"
#include "Components/D3D11Manager.h"
#include "Components/TextureManager.h"
#include "Components/SceneRTManager.h"
// D3D11
#include "D3D11/D3D11State.h"
#include "D3D11/D3D11CoreResources.h"
// Data
#include "Data/RenderTexture.h"
#include "Data/AtmosphereMap.h"
#include "Data/VolumetricCloud.h"
#include "Data/ShadowMap.h"
#include "Data/CloudMap.h"
// Resources
#include "Resources/ConstantBuffer.h"
#include "Resources/VolumeTexture.h"
#include "Resources/Texture.h"
// Post
#include "Post/CloudComposite.h"
#include "Post/WaterComposite.h"
#include "Post/TAA.h"
#include "Post/PostEffects.h"
// Utils
#include "Helpers/ShaderHelper.h"
#include "Helpers/MathHelper.h"
#include "ImGui/ImGuiManager.h"
#include "ImGui/FunctionWidget.h"
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/CameraConstants.h"
#include "SharedConstants/BuffersConstants.h"
#include "SharedConstants/ShadowConstants.h"
#include "SharedConstants/CommonConstants.h"
// define
#define STONE_TRANSFORM_OFFSET 8.0f
#define OBJECT_SHADOW_SLOT     10
#define TERRAIN_SHADOW_SLOT    11
#define SAMPLER_SHADOW_SLOT    5
#define FRAME_CB_SLOT          0
#define DIRL_CB_SLOT           1
#define CLIP_CB_SLOT           5
#define POST_PS_SLOT1          0
#define POST_PS_SLOT2          1

using namespace DirectX;
using namespace SharedConstants;
using namespace PathConstants;
using namespace ConstantBuffer;
using namespace BuffersConstants;
using namespace DebugHelper;
using namespace ShaderHelper;

Renderer::Renderer() {
    m_D3D11Mgr = std::make_unique<D3D11Manager>();
    m_Camera = std::make_unique<Camera>();
	m_SkyBox = std::make_unique<SkyBox>();
    m_Ground = std::make_unique<Ground>();
    m_DirectionalLight = std::make_unique<DirectionalLight>();
	m_VolumetricCloud = std::make_unique<VolumetricCloud>();
	m_ObjectShadowMap = std::make_unique<ShadowMap>();
	m_TerrainShadowMap = std::make_unique<ShadowMap>();
	m_CloudMapLUT = std::make_unique<CloudMap>();
	m_AtmosphereLUT = std::make_unique<AtmosphereMap>();
	m_Composite = std::make_unique<CloudComposite>();
    m_Post = std::make_unique<PostEffects>();
    m_TAA = std::make_unique<TAA>();
	m_Grass = std::make_unique<Grass>();
    m_Tree = std::make_unique<Tree>();
	m_Stone = std::make_unique<MayaActor>();
	m_StonePillar = std::make_unique<MayaActor>();
	m_Arca = std::make_unique<MayaActor>();
	m_Rakshasa = std::make_unique<SkinnedActor>();
    m_LowpolyPlayer = std::make_unique<RigidActor>();
    m_WaterComposite = std::make_unique<WaterComposite>();
    m_Water = std::make_unique<Water>();
    m_TextureMgr = std::make_shared<TextureManager>();
    m_sceneRTMgr = std::make_unique<SceneRTManager>();
    m_nullRTV = nullptr;
    m_nullSRV = nullptr;
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
    auto pointCampSampler = m_D3D11Mgr->GetStates()->GetPointClampSamplerState();

    InitCommonBuffer(device);

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

	InitDefaultMaya(hwnd, device, context, linerWrapSampler);

    CloudMap::InitParams cloudMapParams;
    cloudMapParams.device = device;
    cloudMapParams.hwnd = hwnd;
    if (!m_CloudMapLUT->Init(cloudMapParams)) {
        return false;
    }
    else {
        m_CloudMapLUT->Generate(context);
    }

	AtmosphereMap::InitParams atmoParams;
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
	groundInitParams.heightMapTex = m_TextureMgr->GetTexture(device, context, PathConstants::HEIGHT, true);
	groundInitParams.colSRV = m_TextureMgr->GetTexture(device, context, PathConstants::GROUND_COL)->GetSRV();
	groundInitParams.norSRV = m_TextureMgr->GetTexture(device, context, PathConstants::GROUND_NOR)->GetSRV();
	groundInitParams.linearSampler = linerWrapSampler;

    if (!m_Ground->Init(groundInitParams)) {
        return false;
    }

    Water::InitParams waterInitParams;
    waterInitParams.device = device;
    waterInitParams.hwnd = hwnd;
    waterInitParams.waterHeight = CommonConstants::WATER_HEIGHT;
    waterInitParams.screenWidth = RendererState::ScreenWidth;
    waterInitParams.screenHeight = RendererState::ScreenHeight;
    waterInitParams.waterNormalSRV = m_TextureMgr->GetTexture(device, context, PathConstants::WATER_NOR)->GetSRV();
    waterInitParams.waterWaveNormalSRV = m_TextureMgr->GetTexture(device, context, PathConstants::WATER_WAVE_NOR)->GetSRV();
    waterInitParams.flowSRV = m_TextureMgr->GetTexture(device, context, PathConstants::FLOW_MAP)->GetSRV();
    waterInitParams.linearWrapSampler = linerWrapSampler;
    if (!m_Water->Init(waterInitParams)) {
        return false;
    }

	VolumetricCloud::InitParams cloudInitParams;
	cloudInitParams.device = device;
	cloudInitParams.hwnd = hwnd;
	cloudInitParams.cloudMapLUTSRV = m_CloudMapLUT->GetSRV();
	cloudInitParams.worleyNoiseSRV = m_TextureMgr->GetVolumeTexture(PathConstants::KEY_WORLEY_NOISE)->GetSRV();
	cloudInitParams.blueNoiseSRV = m_TextureMgr->GetTexture(device, context, PathConstants::BLUE_NOISE)->GetSRV();
	cloudInitParams.wrapSampler = linerWrapSampler;
    cloudInitParams.pointSampler = pointCampSampler;

    if (!m_VolumetricCloud->Init(cloudInitParams)) {
        return false;
    }

    ShadowMap::InitParams shadowParams;
    shadowParams.device = device;
    shadowParams.hwnd = hwnd;
    if (!m_ObjectShadowMap->Init(shadowParams) || !m_TerrainShadowMap->Init(shadowParams)) {
        return false;
    }

    if (!m_sceneRTMgr->Init(device, RendererState::ScreenWidth, RendererState::ScreenHeight)) {
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

    PostEffects::InitParams postParams;
    postParams.device = device;
    postParams.hwnd = hwnd;
    postParams.noiseSRV = m_TextureMgr->GetTexture(device, context, PathConstants::NOISE_2D)->GetSRV();
    postParams.depthSRV = m_D3D11Mgr->GetDepthSRV();
    postParams.ScreenWidth = RendererState::ScreenWidth;
    postParams.ScreenHeight = RendererState::ScreenHeight;
    if (!m_Post->Init(postParams)) {
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

    InitWidgets();
    return true;
} // Init

void Renderer::Shutdown() {
    if (m_ImGuiMgr) {
        m_ImGuiMgr.reset();
    }

    // 후처리 및 포스트 이펙트
    if (m_TAA) {
        m_TAA.reset();
    }
    if (m_Post) {
        m_Post.reset();
    }
    if (m_Composite) {
        m_Composite.reset();
    }

    // 렌더 타겟 및 그림자
    if (m_sceneRTMgr) {
        m_sceneRTMgr.reset();
    }
    if (m_TerrainShadowMap) {
        m_TerrainShadowMap.reset();
    }
    if (m_ObjectShadowMap) {
        m_ObjectShadowMap.reset();
    }

    // 볼류매트릭 및 대기 렌더링 객체
    if (m_VolumetricCloud) {
        m_VolumetricCloud.reset();
    }
    if (m_CloudMapLUT) {
        m_CloudMapLUT.reset();
    }
    if (m_AtmosphereLUT) {
        m_AtmosphereLUT.reset();
    }
    if (m_SkyBox) {
        m_SkyBox.reset();
    }

    if (m_Ground) {
        m_Ground.reset();
    }

    if (m_Stone) {
        m_Stone.reset();
    }

    // 렌더러 핵심 컴포넌트 및 매니저
    if (m_TextureMgr) {
        m_TextureMgr.reset();
    }
    if (m_Camera) {
        m_Camera.reset();
    }
    if (m_DirectionalLight) {
        m_DirectionalLight.reset();
    }

    // 상수 버퍼
    if (m_lightBuffer) {
        m_lightBuffer.Reset();
    }
    if (m_frameBuffer) {
        m_frameBuffer.Reset();
    }

    // 그래픽스 API 코어
    if (m_D3D11Mgr) {
        m_D3D11Mgr.reset();
    }
} // Renderer

bool Renderer::Frame(float deltaTime) {
    m_renderingTime += deltaTime;
    m_Camera->Update();
    m_DirectionalLight->Update();

    UpdateModelTransform();
    
    return Render(deltaTime);
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

bool Renderer::Render(float deltaTime) {
    auto context = m_D3D11Mgr->GetDeviceContext();
    auto states  = m_D3D11Mgr->GetStates();

    ShadowPass(context, states);
    MainPass(context, states);
    CompositePass(context, states);
    //WaterPass(context, states);
    PostProcessing(context, states);

    m_ImGuiMgr->Frame();
    m_D3D11Mgr->EndScene(RendererState::VsyncEnable);
    return true;
} // Render

void Renderer::UpdateModelTransform() {
    if (!m_Stone || !m_Ground) {
        return;
    }
    
    XMFLOAT3 pos = m_Stone->GetPosition();
    float terrainY = m_Ground->GetHeightAt(pos.x, pos.z);
    m_Stone->SetPosition(pos.x, terrainY + STONE_TRANSFORM_OFFSET, pos.z);
} // UpdateModelTransform

void Renderer::ShadowPass(ID3D11DeviceContext* context, D3D11State* states) {
    context->OMSetRenderTargets(1, &m_nullRTV, m_ObjectShadowMap->GetDSV());
    m_ObjectShadowMap->ClearShadowDepth(context);
    context->RSSetViewports(1, &m_ObjectShadowMap->GetViewport());

	ShadowMap::RenderParams renderParams;

    DirectX::XMFLOAT3 shadowFocus = XMFLOAT3(0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&m_Stone->GetPosition());
    DirectX::XMStoreFloat3(&shadowFocus, p2);

    m_DirectionalLight->UpdateObjectShadow(shadowFocus);

    DirectX::XMMATRIX sharedView = m_DirectionalLight->GetObjectViewMatrix();
    DirectX::XMMATRIX sharedProj = m_DirectionalLight->GetObjectProjection();

    if (m_Stone) {
        renderParams.viewMatrix = sharedView;
        renderParams.projectionMatrix = sharedProj;
        renderParams.isSkinned = false;
        renderParams.worldMatrix = m_Stone->GetWorldMatrix();

        m_ObjectShadowMap->RenderOpaque(context, renderParams);
        m_Stone->DrawIndexed(context);
    }

    context->OMSetRenderTargets(1, &m_nullRTV, m_TerrainShadowMap->GetDSV());
    m_TerrainShadowMap->ClearShadowDepth(context);
    context->RSSetViewports(1, &m_TerrainShadowMap->GetViewport());

    if (m_Ground) {
        renderParams.viewMatrix = m_DirectionalLight->GetViewMatrix();
        renderParams.projectionMatrix = m_DirectionalLight->GetProjection();
        renderParams.worldMatrix = m_Ground->GetWorldMatrix();
        renderParams.isSkinned = false;
        m_TerrainShadowMap->RenderOpaque(context, renderParams);
        m_Ground->DrawIndexed(context);
    }

    context->OMSetRenderTargets(0, nullptr, nullptr);
    context->PSSetShaderResources(OBJECT_SHADOW_SLOT, 1, &m_nullSRV);
    context->PSSetShaderResources(TERRAIN_SHADOW_SLOT, 1, &m_nullSRV);
} // ShadowPass
void Renderer::MainPass(ID3D11DeviceContext* context, D3D11State* states) {
    m_D3D11Mgr->RestoreViewport();

    float clearColor[4] = { 0.15f, 0.15f, 0.15f, 1.0f };
    float clearNormal[4] = { 0.5f, 0.5f, 1.0f, 1.0f };

    ID3D11RenderTargetView* rtvList[2];
    rtvList[0] = m_sceneRTMgr->GetRT(KEY_SCENE_RT)->GetRTV();
    rtvList[1] = m_sceneRTMgr->GetRT(KEY_NORMAL_RT)->GetRTV();
    context->OMSetRenderTargets(2, rtvList, m_D3D11Mgr->GetDepthRT()->GetDSV());

    context->ClearRenderTargetView(rtvList[0], clearColor);
    context->ClearRenderTargetView(rtvList[1], clearNormal);

    m_D3D11Mgr->GetDepthRT()->ClearDepth(context, 1.0f, 0);

    UpdateCommonShaderBuffer(context, states);
 
    DrawGround(context, states);
    DrawModel(context, states);
    DrawSkyBox(context, states);

	ComputeShaderData(context, states);
} // MainPass

void Renderer::WaterPass(ID3D11DeviceContext* context, D3D11State* states) {
    if (!m_Water) {
        return;
    }

    ID3D11Texture2D* sceneTex = m_sceneRTMgr->GetRT(KEY_SCENE_RT)->GetTexture();
    ID3D11Texture2D* refractionTex = m_sceneRTMgr->GetRT(KEY_REFRACTION_RT)->GetTexture();
    context->CopyResource(refractionTex, sceneTex);

    ID3D11RenderTargetView* sceneRTV = m_sceneRTMgr->GetRT(KEY_SCENE_RT)->GetRTV();
    context->OMSetRenderTargets(1, &sceneRTV, m_D3D11Mgr->GetDepthRT()->GetDSV());

    context->RSSetState(states->GetCullBackState());
    //context->RSSetState(states->GetCullNone());
    context->OMSetDepthStencilState(states->GetDepthState(), 1);
    context->OMSetBlendState(states->GetBlendState(), nullptr, 0xffffffff);

    //FrameBuffer fData;
    //fData.view = XMMatrixTranspose(m_Camera->GetViewMatrix());
    //fData.projection = XMMatrixTranspose(m_Camera->GetProjectionMatrix());
    //fData.viewInv = XMMatrixTranspose(XMMatrixInverse(nullptr, m_Camera->GetViewMatrix()));
    //fData.projInv = XMMatrixTranspose(XMMatrixInverse(nullptr, m_Camera->GetProjectionMatrix()));
    //fData.cameraPosition = m_Camera->GetPosition();
    //fData.time = m_renderingTime;

    //if (UpdateConstantBuffer(context, m_frameBuffer.Get(), fData)) {
    //    context->VSSetConstantBuffers(FRAME_CB_SLOT, 1, m_frameBuffer.GetAddressOf());
    //    context->PSSetConstantBuffers(FRAME_CB_SLOT, 1, m_frameBuffer.GetAddressOf());
    //}

    Water::RenderParams waterParams;
    waterParams.world = m_Water->GetWorldMatrix(m_Camera->GetPosition());
    waterParams.sceneDepthSRV = m_D3D11Mgr->GetDepthRT()->GetSRV();
    waterParams.normalSRV = m_sceneRTMgr->GetRT(KEY_REFLECTION_RT)->GetSRV();
    waterParams.sceneSRV = m_Composite->GetSRV();

    m_Water->Render(context, waterParams);

    context->OMSetBlendState(states->GetNoBlendState(), nullptr, 0xffffffff);
} // WaterPass

void Renderer::PostProcessing(ID3D11DeviceContext* context, D3D11State* states) {
    ApplyEffects(context, states);
    ApplyTAA(context, states);
} // PostProcessing

void Renderer::UpdateCommonShaderBuffer(ID3D11DeviceContext* context, D3D11State* states) {
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
    lData.sunset = m_DirectionalLight->GetSunset();
    lData.night = m_DirectionalLight->GetLight();
    lData.lightViewMatrix = XMMatrixTranspose(m_DirectionalLight->GetViewMatrix());
    lData.lightProjectionMatrix = XMMatrixTranspose(m_DirectionalLight->GetProjection());
    lData.objectViewMatrix = XMMatrixTranspose(m_DirectionalLight->GetObjectViewMatrix());
    lData.objectProjectionMatrix = XMMatrixTranspose(m_DirectionalLight->GetObjectProjection());
    lData.shadowMapWidth = BuffersConstants::SHADOWMAP_WIDTH;
    lData.shadowMapHeight = BuffersConstants::SHADOWMAP_HEIGHT;
    lData.shadowSpread = BuffersConstants::SPREAD;
    lData.shadowBias = BuffersConstants::BIAS;

    if (!UpdateConstantBuffer(context, m_frameBuffer.Get(), fData)) {
        DebugPrint("프레이버퍼 확인 필요");
    }
    if (!UpdateConstantBuffer(context, m_lightBuffer.Get(), lData)) {
        DebugPrint("방향광 버퍼 확인 필요");
    }

    ID3D11ShaderResourceView* objectShadowSRV = m_ObjectShadowMap->GetSRV();
    ID3D11ShaderResourceView* terrainShadowSRV = m_TerrainShadowMap->GetSRV();
	ID3D11SamplerState* shadowSampler = states->GetShadowSamplerState();

    context->PSSetShaderResources(OBJECT_SHADOW_SLOT, 1, &objectShadowSRV);
    context->PSSetShaderResources(TERRAIN_SHADOW_SLOT, 1, &terrainShadowSRV);
    context->PSSetSamplers(SAMPLER_SHADOW_SLOT, 1, &shadowSampler);

    // Slot 0: Frame, Slot 1: Light
    context->VSSetConstantBuffers(FRAME_CB_SLOT, 1, m_frameBuffer.GetAddressOf());
    context->VSSetConstantBuffers(DIRL_CB_SLOT, 1, m_lightBuffer.GetAddressOf());
    context->PSSetConstantBuffers(FRAME_CB_SLOT, 1, m_frameBuffer.GetAddressOf());
    context->PSSetConstantBuffers(DIRL_CB_SLOT, 1, m_lightBuffer.GetAddressOf());
    context->CSSetConstantBuffers(FRAME_CB_SLOT, 1, m_frameBuffer.GetAddressOf());
    context->CSSetConstantBuffers(DIRL_CB_SLOT, 1, m_lightBuffer.GetAddressOf());
    context->GSSetConstantBuffers(FRAME_CB_SLOT, 1, m_frameBuffer.GetAddressOf());
    context->GSSetConstantBuffers(DIRL_CB_SLOT, 1, m_lightBuffer.GetAddressOf());
} // UpdateCommonShaderBuffer

void Renderer::DrawGround(ID3D11DeviceContext* context, D3D11State* states) {
    if (!m_Ground) {
        return;
    }

    context->RSSetState(states->GetCullBackState());
    context->OMSetDepthStencilState(states->GetDepthState(), 1);
    context->OMSetBlendState(states->GetBlendState(), nullptr, 0xffffffff);

    Ground::RenderParams groundParams;
    groundParams.cameraPosition = m_Camera->GetPosition();
    groundParams.time = m_renderingTime;
	groundParams.frustum = m_Camera->GetFrustum();

    m_Ground->Render(context, groundParams);
} // DrawGround


void Renderer::DrawModel(ID3D11DeviceContext* context, D3D11State* states) {
    context->RSSetState(states->GetCullBackState());
    context->OMSetDepthStencilState(states->GetDepthState(), 1);
    context->OMSetBlendState(states->GetNoBlendState(), nullptr, 0xffffffff);

    MayaActor::RenderParams mayaParams;
    mayaParams.world = m_Stone->GetWorldMatrix();
    m_Stone->Render(context, mayaParams);
} // DrawModel

void Renderer::DrawSkyBox(ID3D11DeviceContext* context, D3D11State* states, bool isReflection) {
    if (!m_Camera || !m_SkyBox) {
        return;
    }

    context->RSSetState(states->GetCullNone());
    context->OMSetDepthStencilState(states->GetDepthLessEqual(), 1);
    context->OMSetBlendState(states->GetBlendState(), nullptr, 0xffffffff);

    SkyBox::RenderParams skyParams;
	skyParams.camPos = m_Camera->GetPosition();
    skyParams.time = m_renderingTime;
	skyParams.skyLUT = m_AtmosphereLUT->GetLUT();
    skyParams.isReflection = isReflection ? 1 : 0;

    m_SkyBox->Render(context, skyParams);
    context->OMSetBlendState(states->GetNoBlendState(), nullptr, 0xffffffff);
} // DrawSkyBox

void Renderer::DrawGrass(ID3D11DeviceContext* context, D3D11State* states) {
    if (!m_Grass || !m_Camera || !m_Ground) {
        return;
    }
    context->RSSetState(states->GetCullNone());
    context->OMSetDepthStencilState(states->GetDepthState(), 1);

    Grass::RenderParams grassParams;
    grassParams.frustum = m_Camera->GetFrustum();
    grassParams.visibleNodes = &m_Ground->GetVisibleNodes();

    m_Grass->Render(context, grassParams);
    m_Grass->RenderFar(context, grassParams);
} // DrawGrass

void Renderer::ComputeShaderData(ID3D11DeviceContext* context, D3D11State* states) {
    AtmosphereMap::ExecuteParams atmoExecParams;
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

void Renderer::ApplyWater(ID3D11DeviceContext* context, D3D11State* states) {
    context->PSSetShaderResources(POST_PS_SLOT1, 1, &m_nullSRV);
    context->PSSetShaderResources(POST_PS_SLOT2, 1, &m_nullSRV);

    ID3D11RenderTargetView* compositeRTV = m_WaterComposite->GetRTV();
    context->OMSetRenderTargets(1, &compositeRTV, nullptr);
    m_WaterComposite->ClearRT(context);
    
    WaterComposite::RenderParams waterParams;
    waterParams.sceneSRV = m_sceneRTMgr->GetRT(KEY_SCENE_RT)->GetSRV();
    waterParams.sceneDepthSRV = m_D3D11Mgr->GetDepthRT()->GetSRV();
    
    m_WaterComposite->Render(context, waterParams);
} // ApplyWater

void Renderer::CompositePass(ID3D11DeviceContext* context, D3D11State* states) {
    context->PSSetShaderResources(POST_PS_SLOT1, 1, &m_nullSRV);
    context->PSSetShaderResources(POST_PS_SLOT2, 1, &m_nullSRV);

    ID3D11RenderTargetView* compositeRTV = m_Composite->GetRTV();
    context->OMSetRenderTargets(1, &compositeRTV, nullptr);
    m_Composite->ClearRT(context);

    CloudComposite::RenderParams renderParam;
    //renderParam.sceneSRV = m_WaterComposite->GetSRV();
    renderParam.sceneSRV = m_sceneRTMgr->GetRT(KEY_SCENE_RT)->GetSRV();
    renderParam.cloudSRV = m_VolumetricCloud->GetCloudSRV();
	renderParam.depthSRV = m_D3D11Mgr->GetDepthSRV();
    renderParam.linerSampler = states->GetLinearWrapSamplerState();
    m_Composite->Render(context, renderParam);
} // CompositePass

void Renderer::ApplyEffects(ID3D11DeviceContext* context, D3D11State* states) {
    context->PSSetShaderResources(POST_PS_SLOT1, 1, &m_nullSRV);
    context->PSSetShaderResources(POST_PS_SLOT2, 1, &m_nullSRV);

    ID3D11RenderTargetView* postRTV = m_Post->GetRTV();
    context->OMSetRenderTargets(1, &postRTV, nullptr);
    m_Post->ClearRT(context);

    PostEffects::RenderParams effectParam;
    effectParam.inputSRV = m_Composite->GetSRV();
    effectParam.cloudSRV = m_VolumetricCloud->GetCloudSRV();
    effectParam.transmittanceSRV = m_VolumetricCloud->GetTransmittanceSRV();
    effectParam.linerSampler = states->GetLinearWrapSamplerState();
    effectParam.lightUV = m_DirectionalLight->GetUV(m_Camera->GetViewMatrix(), m_Camera->GetProjectionMatrix());
    m_Post->Render(context, effectParam);
} // ApplyEffects

void Renderer::ApplyTAA(ID3D11DeviceContext* context, D3D11State* states) {
    static bool isFirst = true;

    context->PSSetShaderResources(POST_PS_SLOT1, 1, &m_nullSRV);
    context->PSSetShaderResources(POST_PS_SLOT2, 1, &m_nullSRV);

    ID3D11RenderTargetView* backRTV = m_D3D11Mgr->GetRTV();
    context->OMSetRenderTargets(1, &backRTV, nullptr);

    TAA::RenderParams taaParam;
    taaParam.preViewProj = m_Camera->GetViewMatrix() * m_Camera->GetProjectionMatrix();
    taaParam.currentSRV = m_Post->GetSRV();
    taaParam.depthSRV = m_D3D11Mgr->GetDepthSRV();
    taaParam.linerSampler = states->GetLinearWrapSamplerState();
    taaParam.blendFactor = isFirst ? 0.0f : 0.85f;
    taaParam.texelSize = XMFLOAT2(1.0f / RendererState::ScreenWidth,1.0f / RendererState::ScreenHeight);
    m_TAA->Render(context, taaParam);

    isFirst = false;
    m_TAA->CopyResource(context, m_Post->GetTexture());
} // ApplyTAA

void Renderer::InitCommonBuffer(ID3D11Device* device) {
    if (!InitConstantBuffer<FrameBuffer>(device, m_frameBuffer.GetAddressOf())) {
        return;
    }

    if (!InitConstantBuffer<DirectionalLightBuffer>(device, m_lightBuffer.GetAddressOf())) {
        return;
    }
\
} // InitCommonBuffer

void Renderer::InitDefaultMaya(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context, ID3D11SamplerState* linerWrapSampler) {
    if (!m_Stone || !m_StonePillar || !m_Arca) {
        return;
    }

    MayaActor::InitParams initParams;
    initParams.device = device;
    initParams.context = context;
    initParams.hwnd = hwnd;
    initParams.textMgr = m_TextureMgr;
    initParams.VSPath = PBR_VS;
    initParams.linerSampler = linerWrapSampler;

    initParams.path = STONE;
    initParams.PSPath = STONE_PS;
    if (!m_Stone->Init(initParams)) {
        return;
    }
    else {
        m_Stone->SetPosition(20.0f, 0.0f, 20.0f);
		m_Stone->SetScale(10.0f, 10.0f, 10.0f);
    }
} // InitDefaultMaya

void Renderer::OnGui() {
    ImGui::Text("Renderer RT");
    ImGui::Separator();

    if (auto normalRT = m_sceneRTMgr->GetRT(KEY_NORMAL_RT)) {
        ImGui::Image((ImTextureID)normalRT->GetSRV(), ImVec2(256, 144));
    }
} // onGui

void Renderer::InitWidgets() {
    if (m_ImGuiMgr) {
        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "Camera Control",
            [this]() { m_Camera->OnGui(); }
        ));

        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "Directional Light Control",
            [this]() { m_DirectionalLight->OnGui(); }
        ));

        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "Volumetric Cloud Control",
            [this]() { m_VolumetricCloud->OnGui(); }
        ));

        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "RT Control",
            [this]() { this->OnGui(); }
        ));

        m_ImGuiMgr->AddWidget(std::make_unique<FunctionWidget>(
            "WaterComposite Control",
            [this]() { m_WaterComposite->OnGui(); }
        ));

    }
} // InitWidgets