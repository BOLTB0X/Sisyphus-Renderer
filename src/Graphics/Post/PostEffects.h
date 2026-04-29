#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <directxmath.h>
#include <memory>
#include "SharedConstants/ScreenConstants.h"
#include "SharedConstants/BuffersConstants.h"

class RenderTexture;

class PostEffects {
public:
    struct InitParams {
        ID3D11Device*             device;
        HWND                      hwnd;
        ID3D11ShaderResourceView* noiseSRV;
        ID3D11ShaderResourceView* depthSRV;
        int                       ScreenWidth;
        int		                  ScreenHeight;
        InitParams() : device(nullptr), hwnd(nullptr),
            noiseSRV(nullptr), depthSRV(nullptr),
            ScreenWidth(0), ScreenHeight(0) {
        }
    }; // InitParams

    struct RenderParams {
        ID3D11ShaderResourceView* inputSRV;
        ID3D11ShaderResourceView* cloudSRV;
        ID3D11SamplerState*       linerSampler;
        DirectX::XMFLOAT2         lightUV;
        RenderParams() : inputSRV(nullptr), cloudSRV(nullptr), linerSampler(nullptr)
        , lightUV(0.0f, 0.0f) {
        }
    }; // RenderParams

public:
    PostEffects();
    ~PostEffects();
    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void OnGui();

    void                      ClearRT(ID3D11DeviceContext*);
    ID3D11Texture2D*          GetTexture() const;
    ID3D11RenderTargetView*   GetRTV() const;
    ID3D11ShaderResourceView* GetSRV() const;

private:
    struct GodRayBuffer {
        DirectX::XMFLOAT2 lightUV;
        float             density;
        float             weight;

        float             decay;
        float             exposure;
        float             luminanceThreshold;
        float             padding;

        GodRayBuffer() {
            lightUV = { 0.5f, 0.5f };
            density = 0.8f;
            weight = 0.05f;
            decay = 0.85f;
            exposure = 1.0f;
            luminanceThreshold = 0.8f;
            padding = 0.0f;
        }
    }; // GodRayBuffer

    struct LensFlareBuffer {
        // Row 1
        DirectX::XMFLOAT3 retouchColor;
        float             lensScale;
        // Row 2
        DirectX::XMFLOAT2 coreUV;
        float             coreTight;
        float             luminanceOffest;

        LensFlareBuffer() {
            retouchColor = SharedConstants::BuffersConstants::LENSFLARE_RETOUCH_COLOR;
            lensScale = 0.015f;
            coreUV = { 0.0f, 0.0f };
            coreTight = 256.0f;
            luminanceOffest = 0.8f;
        }
    }; // LensFlareBuffer

private:
    void ApplyBloom(ID3D11DeviceContext*, ID3D11ShaderResourceView*);
    void ApplyGodRays(ID3D11DeviceContext*, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, DirectX::XMFLOAT2);
    void ApplyLensflare(ID3D11DeviceContext*, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, DirectX::XMFLOAT2);
    void ApplyComposite(ID3D11DeviceContext*);

private:
    std::unique_ptr<RenderTexture>             m_resRT;
    std::unique_ptr<RenderTexture>             m_bloomRT;
    std::unique_ptr<RenderTexture>             m_rayRT;
    std::unique_ptr<RenderTexture>             m_flareRT;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_bloomShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_lensFlareShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_godRayShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_compositeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_lensflareBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_godRayBuffer;

    ID3D11ShaderResourceView*                  m_noiseSRV;
    ID3D11ShaderResourceView*                  m_depthSRV;
    LensFlareBuffer                            m_lensflareData;
    GodRayBuffer                               m_godRayData;
}; // PostEffects