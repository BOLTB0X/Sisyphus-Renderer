#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <directxmath.h>
#include <memory>

class RenderTexture;

class Bloom {
public:
    struct InitParams {
        ID3D11Device* device;
        HWND         hwnd;
        std::wstring  vPath;
        std::wstring  pPath;
        int           ScreenWidth;
        int		      ScreenHeight;
        InitParams() : device(nullptr), hwnd(nullptr),
            vPath(L""), pPath(L""),
            ScreenWidth(0), ScreenHeight(0) {
        }
    }; // InitParams

    struct RenderParams {
        ID3D11ShaderResourceView* inputSRV;
        ID3D11SamplerState*       linerSampler;
        RenderParams() : inputSRV(nullptr), linerSampler(nullptr) {
        }
    }; // RenderParams

public:
    Bloom();
    ~Bloom();
    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);

    void                      ClearRT(ID3D11DeviceContext*);
    ID3D11Texture2D*          GetTexture() const;
    ID3D11RenderTargetView*   GetRTV() const;
    ID3D11ShaderResourceView* GetSRV() const;

private:
    std::unique_ptr<RenderTexture>             m_bloomRT;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
}; // Bloom