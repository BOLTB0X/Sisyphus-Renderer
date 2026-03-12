#include "Pch.h"
#include "Renderer.h"
#include "RendererState.h"
// Objects
#include "Objects/Triangle.h"
#include "Objects/Stone.h"
#include "Objects/SkyBox.h"
// D3D11
#include "D3D11/D3D11Manager.h"
#include "D3D11/D3D11State.h"
#include "D3D11/D3D11CoreResources.h"
// Camera
#include "Camera/Camera.h"
// Resources
#include "Resources/TextureManager.h"
#include "Resources/VolumeTexture.h"
#include "Resources/ConstantBufferType.h"
#include "Resources/NoiseGenerator.h"
#include "Resources/VolumetricCloud.h"
// Shaders
// Utils
#include "ImGui/ImGuiManager.h"
#include "ImGui/ImGuiDrawer.h"
#include "ImGui/CameraWidget.h"
#include "ImGui/AssimpModelWidget.h"
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/CameraConstants.h"
#include "Helpers/DebugHelper.h"

using namespace SharedConstants;
using namespace PathConstants;
using namespace ConstantBuffer;
using namespace DebugHelper;

Renderer::Renderer() {
    m_D3D11Mgr = std::make_unique<D3D11Manager>();
    m_Triangle = std::make_unique<Triangle>();
    m_Stone = std::make_unique<Stone>();
    m_Camera = std::make_unique<Camera>();
    m_VolumeTexture = std::make_unique<VolumeTexture>();
    m_NoiseGenerator = std::make_unique<NoiseGenerator>();
	m_SkyBox = std::make_unique<SkyBox>();
    m_VolumetricCloud = std::make_unique<VolumetricCloud>();
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

    if (!m_Camera->Init(CameraConstants::DEFAULT_FOV, RendererState::aspectRatio,
                        RendererState::ScreenNear, RendererState::ScreenDepth)) {
        return false;
    }

    if (!m_Stone->Init(device, context, hwnd, m_TextureMgr, STONE)) {
        return false;
    }

    if (!m_VolumeTexture->Init(device, 128, 128, 128, DXGI_FORMAT_R8G8B8A8_UNORM)) {
        return false;
    }

    if (!m_NoiseGenerator->Init(device, hwnd, NOISEGEN_CS)) {
        return false;
    }

    auto sampler = m_D3D11Mgr->GetStates()->GetLinearSamplerState();
    if (!m_SkyBox->Init(device, context, hwnd, sampler)) {
        return false;
	}
    // ImGui 초기화
    m_ImGuiMgr = std::move(imgui);
    if (m_ImGuiMgr && !m_ImGuiMgr->Init(hwnd, device, context)) {
        return false;
    }

    InitWidgets();
    GenerateCloudNoise(context);
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

    static bool bBaked = false;
    if (!bBaked) {
        Atmosphere::RenderParams atmoParams;
        atmoParams.camPos = m_Camera->GetPosition();
        atmoParams.lightDir = { 0.5f, -1.0f, 0.5f };
        atmoParams.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
        m_SkyBox->UpdateAtmosphere(context, states, atmoParams);
        bBaked = true;
    }

    static float s_lastBakeCamY = FLT_MAX;
    const float bakeThresholdY = 1.0f;
    DirectX::XMFLOAT3 camPos = m_Camera->GetPosition();

    bool needBake = false;
    if (s_lastBakeCamY == FLT_MAX) {
        DebugHelper::DebugPrint("첫 Bake 완료.");
        needBake = true; // 첫 프레임
    } else {
        float dy = fabsf(camPos.y - s_lastBakeCamY);
        if (dy > bakeThresholdY) {
            needBake = true;
            DebugHelper::DebugPrint(fmt::format("Bake 실행 Height 변경: {:.2f} -> {:.2f}", s_lastBakeCamY, camPos.y));
        }
    }

    if (needBake) {
        Atmosphere::RenderParams atmoParams;
        atmoParams.camPos = camPos;
        atmoParams.lightDir = { 0.5f, -1.0f, 0.5f };
        atmoParams.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
        m_SkyBox->UpdateAtmosphere(context, states, atmoParams);
        s_lastBakeCamY = camPos.y;
    }

	// 배경 렌더링 (대기권)
    context->RSSetState(states->GetCullNone());
    context->OMSetDepthStencilState(states->GetDepthLessEqual(), 1);

    SkyBox::RenderParams skyParams;
    skyParams.view = m_Camera->GetViewMatrix();
    skyParams.projection = m_Camera->GetProjectionMatrix();
    skyParams.lightDir = { 0.5f, -1.0f, 0.5f };
    m_SkyBox->Render(context, skyParams);

    // 레이 확인
    //VolumetricCloud::RenderParams cloudParams;
    //cloudParams.view = m_Camera->GetViewMatrix();
    //cloudParams.projection = m_Camera->GetProjectionMatrix();
    //cloudParams.camPos = m_Camera->GetPosition();
    //m_VolumetricCloud->Render(context, cloudParams);

    // 불투명 오브젝트 렌더링
 //   context->RSSetState(states->GetCullBackState());
 //   context->OMSetDepthStencilState(states->GetDepthState(), 1);
 //   float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
 //   context->OMSetBlendState(states->GetBlendState(), blendFactor, 0xffffffff);
 //   
	//DrawStone(context, states);

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

void Renderer::GenerateCloudNoise(ID3D11DeviceContext* context) {
    if (!m_NoiseGenerator || !m_VolumeTexture) return;

    NoiseBuffer noiseParams;
    noiseParams.textureSize = { 128.0f, 128.0f, 128.0f };
    noiseParams.perlinFreq = 4.0f;
    noiseParams.worleyFreq = 8.0f;
    noiseParams.detailFreqG = 8.0f;
    noiseParams.detailFreqB = 16.0f;
    noiseParams.detailFreqA = 32.0f;
    noiseParams.octaves = 3;
    noiseParams.remapBias = 0.0f;

    m_NoiseGenerator->Generate(context, m_VolumeTexture.get(), noiseParams);
    DebugPrint("노이즈 굽기 완료");
} // GenerateCloudNoise

void Renderer::DrawStone(ID3D11DeviceContext* context, D3D11State* states) {
    if (!m_Stone) return;

    m_Stone->SetSampler(states->GetLinearSamplerState());

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