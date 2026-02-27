#include "Pch.h"
#include "D3D11Manager.h"
#include "D3D11CoreResources.h"
#include "D3D11State.h"
#include "DisplayInfo.h"
#include "Helpers/DebugHelper.h"

D3D11Manager::D3D11Manager() {
    m_displayInfo = std::make_unique<DisplayInfo>();
    m_core = std::make_unique<D3D11CoreResources>();
    m_state = std::make_unique<D3D11State>();
    m_viewport = {0}; 
} // D3D11Manager

D3D11Manager::D3D11Manager(const D3D11Manager& other) {
    m_displayInfo = std::make_unique<DisplayInfo>();
    m_core = std::make_unique<D3D11CoreResources>();
    m_state = std::make_unique<D3D11State>();
    m_viewport = { 0 };
} // D3D11Manager

D3D11Manager::~D3D11Manager() {
} // ~D3D11Manager

bool D3D11Manager::Init(HWND hwnd, int width, int height, bool fullscreen, bool vsync) {
    if (!m_displayInfo->Init(width, height))
        return false;
    if (!m_core->Init(hwnd, width, height, fullscreen, vsync, 
                      m_displayInfo->GetNumerator(),
                      m_displayInfo->GetDenominator()))
        return false;
    if (!m_state->Init(m_core->GetDevice()))
        return false;

    // 추가된 뷰 초기화 메서드 호출
    if (!InitViews(width, height))
        return false;
    return true;
} // Init

void D3D11Manager::BeginScene(float r, float g, float b, float a) {
    float color[4] = { r, g, b, a };
    auto context = m_core->GetDeviceContext();

    // 화면 및 깊이 버퍼 클리어
    context->ClearRenderTargetView(m_renderTargetView.Get(), color);
    context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 렌더 타겟 바인딩 및 뷰포트 설정
    context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
    context->RSSetViewports(1, &m_viewport);
} // BeginScene

void D3D11Manager::EndScene(bool vsync) {
    m_core->GetSwapChain()->Present(vsync ? 1 : 0, 0);
} // EndScene

bool D3D11Manager::InitViews(int width, int height) {
    if (!InitRenderTargetView())
        return false;
    if (!InitDepthStencilView(width, height))
        return false;
    InitViewport(width, height);
    
    return true;
} // InitViews

bool D3D11Manager::InitRenderTargetView() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    
    // 스왑체인에서 백버퍼 텍스처를 가져와서 RTV 생성
    HRESULT hr = m_core->GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr)) {
        DebugHelper::DebugPrint("백버퍼 획득 실패");
        return false;
    }

    hr = m_core->GetDevice()->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    if ((FAILED(hr))) {
        DebugHelper::DebugPrint("Render Target View 생성 실패");
        return false;
    }
    return true;
} // InitRenderTargetView

bool D3D11Manager::InitDepthStencilView(int width, int height) {
    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    depthBufferDesc.Width = width;
    depthBufferDesc.Height = height;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24비트 깊이, 8비트 스텐실
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    HRESULT hr = m_core->GetDevice()->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthStencilBuffer);
    if (FAILED(hr)) {
        DebugHelper::DebugPrint("Depth Buffer 생성 실패");
        return false ;
    }

    hr = m_core->GetDevice()->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, &m_depthStencilView);
    if (FAILED(hr)) {
        DebugHelper::DebugPrint("Depth Stencil View 생성 실패");
        return false;
    }

    return true;
} // InitDepthStencilView

void D3D11Manager::InitViewport(int width, int height) {
    m_viewport.Width = (float)width;
    m_viewport.Height = (float)height;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
} // InitViewport

ID3D11Device*        D3D11Manager::GetDevice() const  { return m_core->GetDevice(); }
ID3D11DeviceContext* D3D11Manager::GetDeviceContext() const { return m_core->GetDeviceContext(); }
D3D11State*          D3D11Manager::GetStates() const { return m_state.get(); }