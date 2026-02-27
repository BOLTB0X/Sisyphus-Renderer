#include "Pch.h"
#include "Renderer.h"
#include "RendererState.h"
// Objects
#include "Objects/Triangle.h"
// D3D11
#include "D3D11/D3D11Manager.h"
#include "D3D11/D3D11State.h"
#include "D3D11/D3D11CoreResources.h"
// Utils
#include "ImGui/ImGuiManager.h"

Renderer::Renderer() {
    m_D3D11Manager = std::make_unique<D3D11Manager>();
    m_Triangle = std::make_unique<Triangle>();
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

    if (!m_Triangle->Init(m_D3D11Manager->GetDevice(), hwnd)) {
        return false;
    }

    m_ImGuiManager = std::move(imgui);
    if (m_ImGuiManager && !m_ImGuiManager->Init(hwnd, m_D3D11Manager->GetDevice(), m_D3D11Manager->GetDeviceContext())) {
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

    // 파이프라인 조립에 필요한 객체들 획득
    auto context = m_D3D11Manager->GetDeviceContext();
    auto states  = m_D3D11Manager->GetStates();

    // GPU 상태 세팅
    context->RSSetState(states->GetRasterizerState());
    context->OMSetDepthStencilState(states->GetDepthStencilState(), 1);
    context->OMSetBlendState(states->GetBlendState(), nullptr, 0xffffffff);
    
    ID3D11SamplerState* sampler = states->GetLinearSamplerState();
    context->PSSetSamplers(0, 1, &sampler);

    m_Triangle->Render(context);

    if (m_ImGuiManager) {
        m_ImGuiManager->Frame(); 
    }

    m_D3D11Manager->EndScene(RendererState::VsyncEnable);

    return true;
} // Render
