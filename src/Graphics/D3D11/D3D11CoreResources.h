#pragma once
#include <d3d11.h>
#include <wrl/client.h>


// 장치, 컨텍스트, 스왑체인 그릇
class D3D11CoreResources {
public:
    D3D11CoreResources();
    D3D11CoreResources(const D3D11CoreResources&);
    ~D3D11CoreResources();

    bool Init(HWND, int, int, bool, bool, unsigned int, unsigned int);

    ID3D11Device*        GetDevice() const;
    ID3D11DeviceContext* GetDeviceContext() const;
    IDXGISwapChain*      GetSwapChain() const;

private:
    void FillSwapChainDesc(DXGI_SWAP_CHAIN_DESC&, HWND, int, int, 
                           bool, bool, unsigned int, unsigned int);
    bool CreateDeviceAndSwapChain(const DXGI_SWAP_CHAIN_DESC&);

private:
    Microsoft::WRL::ComPtr<ID3D11Device>        m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain>      m_swapChain;
}; // DX11Device