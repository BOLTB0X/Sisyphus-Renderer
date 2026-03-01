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
// Utils
#include "ImGui/ImGuiManager.h"
#include "SharedConstants/PathConstants.h"

using namespace SharedConstants;

Renderer::Renderer() {
    m_D3D11Manager = std::make_unique<D3D11Manager>();
    m_Triangle = std::make_unique<Triangle>();
    m_Stone = std::make_unique<Stone>();
    m_Camera = std::make_unique<Camera>();
} // Renderer

Renderer::Renderer(const Renderer& other) {
    m_D3D11Manager = std::make_unique<D3D11Manager>();
} // Renderer

Renderer::~Renderer() {
} // ~Renderer

bool Renderer::Init(HWND hwnd, std::shared_ptr<ImGuiManager> imgui) {
    if (!m_D3D11Manager->Init(hwnd, RendererState::ScreenWidth, RendererState::ScreenHeight,
        RendererState::FullScrren, RendererState::VsyncEnable)) {
        return false;
    }

    auto device = m_D3D11Manager->GetDevice();
    auto context = m_D3D11Manager->GetDeviceContext();

    float aspect = (float)RendererState::ScreenWidth / (float)RendererState::ScreenHeight;
    if (!m_Camera->Init(60.0f, aspect, RendererState::ScreenNear, RendererState::ScreenDepth)) {
        return false;
    }
    m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

    // Stone 초기화 
    if (!m_Stone->Init(device, context, hwnd, PathConstants::STONE)) {
        return false;
    }
    m_Stone->SetPosition(0.0f, 0.0f, 0.0f);

    // ImGui 초기화
    m_ImGuiManager = std::move(imgui);
    if (m_ImGuiManager && !m_ImGuiManager->Init(hwnd, device, context)) {
        return false;
    }

    return true;
} // Init

void Renderer::Shutdown() {
    if (m_Triangle)
        m_Triangle->Shutdown();
    if (m_D3D11Manager) {
        m_D3D11Manager.reset();
    }
} // Shutdown

bool Renderer::Frame() {
    return Render();
} // Frame


bool Renderer::Render() {
    m_D3D11Manager->BeginScene(0.15f, 0.15f, 0.15f, 1.0f);
    auto context = m_D3D11Manager->GetDeviceContext();
    auto states  = m_D3D11Manager->GetStates();

    float aspect = (float)RendererState::ScreenWidth / (float)RendererState::ScreenHeight;

    m_Camera->Update();

    // GPU 상태 세팅
    context->RSSetState(states->GetRasterizerState());
    context->OMSetDepthStencilState(states->GetDepthStencilState(), 1);
    context->OMSetBlendState(states->GetBlendState(), nullptr, 0xffffffff);
    
    ID3D11SamplerState* sampler = states->GetLinearSamplerState();
    //context->PSSetSamplers(0, 1, &sampler);
    //m_Triangle->Render(context);
    m_Stone->SetSampler(sampler);
    m_Stone->SetPosition(0.0f, 0.0f, 0.0f);

    Stone::RenderParams stoneParams;
    stoneParams.world      = m_Stone->GetWorldMatrix();
    stoneParams.view       = m_Camera->GetViewMatrix();
    stoneParams.projection = m_Camera->GetProjectionMatrix();
    
    stoneParams.camPos     = m_Camera->GetPosition();
    stoneParams.diffuse    = { 1.0f, 1.0f, 1.0f, 1.0f };
    stoneParams.lightDir   = { 0.5f, -1.0f, 0.5f };

    m_Stone->Render(context, stoneParams);

    if (m_ImGuiManager) {
        m_ImGuiManager->Frame(); 
    }

    m_D3D11Manager->EndScene(RendererState::VsyncEnable);

    return true;
} // Render
