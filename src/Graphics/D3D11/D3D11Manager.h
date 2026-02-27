#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
// STL
#include <memory>

class DisplayInfo;
class D3D11CoreResources;
class D3D11State;

class D3D11Manager {
public:
    D3D11Manager();
    D3D11Manager(const D3D11Manager& other);
    ~D3D11Manager();

    bool Init(HWND, int, int, bool, bool);
    
    void BeginScene(float, float, float, float);
    void EndScene(bool);

    ID3D11Device*        GetDevice() const;
    ID3D11DeviceContext* GetDeviceContext() const;
    D3D11State*          GetStates() const;

private:
    bool InitViews(int, int);
    bool InitRenderTargetView();
    bool InitDepthStencilView(int, int);
    void InitViewport(int, int);

private:
    std::unique_ptr<DisplayInfo>        m_displayInfo;
    std::unique_ptr<D3D11CoreResources> m_core;
    std::unique_ptr<D3D11State>         m_state;

    // RenderTarget
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>        m_depthStencilBuffer;
    D3D11_VIEWPORT                                 m_viewport;
}; // D3D11Manager