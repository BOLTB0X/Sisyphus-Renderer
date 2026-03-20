#pragma once
#include <d3d11.h>
#include <wrl/client.h>

class RenderTexture {
public:
    RenderTexture();
    ~RenderTexture();

    bool Init(ID3D11Device* device, int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT);
    bool InitDepth(ID3D11Device* device, int width, int height);

    void Clear(ID3D11DeviceContext*, float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
    void ClearDepth(ID3D11DeviceContext*, float depth = 1.0f, UINT8 stencil = 0);

    // Getters
    ID3D11ShaderResourceView* GetSRV() const;
    ID3D11RenderTargetView*   GetRTV() const;
    ID3D11DepthStencilView*   GetDSV() const;
    int                       GetWidth() const;
    int                       GetHeight() const;

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_texture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_rtv;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   m_dsv;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;

    int m_width;
    int m_height;
}; // RenderTexture