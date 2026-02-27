#include "Pch.h"
#include "D3D11CoreResources.h"
// Utils
#include "Helpers/DebugHelper.h"


D3D11CoreResources::D3D11CoreResources() {
} // D3D11CoreResources

D3D11CoreResources::D3D11CoreResources(const D3D11CoreResources&) {
} // D3D11CoreResources

D3D11CoreResources::~D3D11CoreResources() {
    if (m_swapChain) 
        m_swapChain->SetFullscreenState(false, nullptr);
} // ~D3D11CoreResources

bool D3D11CoreResources::Init(HWND hwnd, int width, int height,
                              bool fullscreen, bool vsync,
                              unsigned int numerator, unsigned int denominator) {
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    
    // 스왑 체인 설정값 채우기
    FillSwapChainDesc(swapChainDesc, hwnd, width, height,
                      fullscreen, vsync, numerator, denominator);
    // 장치 및 스왑 체인 실제 생성
    if (!CreateDeviceAndSwapChain(swapChainDesc))
        return false;

    return true;
} // Init

ID3D11Device*        D3D11CoreResources::GetDevice() const { return m_device.Get(); }
ID3D11DeviceContext* D3D11CoreResources::GetDeviceContext() const { return m_deviceContext.Get(); }
IDXGISwapChain*      D3D11CoreResources::GetSwapChain() const { return m_swapChain.Get(); }

void D3D11CoreResources::FillSwapChainDesc(DXGI_SWAP_CHAIN_DESC& desc, HWND hwnd, int width, int height, 
                                           bool fullscreen, bool vsync, unsigned int num, unsigned int den) {
    ZeroMemory(&desc, sizeof(desc));

    desc.BufferCount = 1;
    desc.BufferDesc.Width = width;
    desc.BufferDesc.Height = height;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    if (vsync) {
        desc.BufferDesc.RefreshRate.Numerator = num;
        desc.BufferDesc.RefreshRate.Denominator = den;
    } else {
        desc.BufferDesc.RefreshRate.Numerator = 0;
        desc.BufferDesc.RefreshRate.Denominator = 1;
    }

    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = hwnd;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Windowed = !fullscreen;

    desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.Flags = 0;
} // FillSwapChainDesc

bool D3D11CoreResources::CreateDeviceAndSwapChain(const DXGI_SWAP_CHAIN_DESC& desc) {
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        &featureLevel,
        1,
        D3D11_SDK_VERSION,
        &desc,
        &m_swapChain,
        &m_device,
        nullptr,
        &m_deviceContext
    );

    if (FAILED(hr)) {
        DebugHelper::DebugPrint("D3D11 장치 및 스왑체인 생성 실패");
        return false;
    }
    return true;
} // CreateDeviceAndSwapChain