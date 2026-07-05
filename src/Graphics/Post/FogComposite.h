#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>

class RenderTexture;

class FogComposite {
public:
    struct InitParams {
        ID3D11Device* device;
        HWND          hwnd;
        int           screenWidth;
        int           screenHeight;

        InitParams() : device(nullptr), hwnd(nullptr),
            screenWidth(0), screenHeight(0) {
        }
    }; // InitParams

    struct RenderParams {
        ID3D11ShaderResourceView* sceneSRV;
        ID3D11ShaderResourceView* fogSRV;

        RenderParams() : sceneSRV(nullptr), fogSRV(nullptr) {
        }
    }; // RenderParams

public:
    FogComposite();
    ~FogComposite();

    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);

    void                      ClearRT(ID3D11DeviceContext*);
    ID3D11Texture2D*          GetTexture() const;
    ID3D11RenderTargetView*   GetRTV() const;
    ID3D11ShaderResourceView* GetSRV() const;

private:
    bool InitShader(ID3D11Device*, HWND);

private:
    std::unique_ptr<RenderTexture>             m_compositeRT;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
}; // FogComposite