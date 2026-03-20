#include "Pch.h"
#include "D3D11Manager.h"
#include "D3D11CoreResources.h"
#include "D3D11State.h"
#include "DisplayInfo.h"
#include "RenderTexture.h"
#include "Helpers/DebugHelper.h"

D3D11Manager::D3D11Manager() {
    m_displayInfo = std::make_unique<DisplayInfo>();
    m_core = std::make_unique<D3D11CoreResources>();
    m_state = std::make_unique<D3D11State>();
    m_depthBuffer = std::make_unique<RenderTexture>();
    m_viewport = {0}; 
} // D3D11Manager

D3D11Manager::D3D11Manager(const D3D11Manager& other) {
    m_displayInfo = std::make_unique<DisplayInfo>();
    m_core = std::make_unique<D3D11CoreResources>();
    m_state = std::make_unique<D3D11State>();
    m_depthBuffer = std::make_unique<RenderTexture>();
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
    m_depthBuffer->ClearDepth(context, 1.0f, 0);
    //context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 렌더 타겟 바인딩 및 뷰포트 설정
    context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthBuffer->GetDSV());
    //context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
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
    if (!m_depthBuffer->InitDepth(m_core->GetDevice(), width, height)) {
        DebugHelper::DebugPrint("RenderTexture를 이용한 뎁스 버퍼 초기화 실패");
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

ID3D11Device*             D3D11Manager::GetDevice() const  { return m_core->GetDevice(); }
ID3D11DeviceContext*      D3D11Manager::GetDeviceContext() const { return m_core->GetDeviceContext(); }
D3D11State*               D3D11Manager::GetStates() const { return m_state.get(); }
ID3D11ShaderResourceView* D3D11Manager::GetDepthSRV() const { return m_depthBuffer->GetSRV(); }
ID3D11RenderTargetView*   D3D11Manager::GetRTV() const { return m_renderTargetView.Get(); }