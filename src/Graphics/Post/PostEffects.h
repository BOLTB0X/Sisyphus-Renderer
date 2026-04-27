#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <directxmath.h>
#include <memory>
#include "SharedConstants/ScreenConstants.h"

class RenderTexture;

class PostEffects {
public:
    struct InitParams {
        ID3D11Device*             device;
        HWND                      hwnd;
        ID3D11ShaderResourceView* noiseSRV;
        int                       ScreenWidth;
        int		                  ScreenHeight;
        InitParams() : device(nullptr), hwnd(nullptr), noiseSRV(nullptr),
            ScreenWidth(0), ScreenHeight(0) {
        }
    }; // InitParams

    struct RenderParams {
        ID3D11ShaderResourceView* inputSRV;
        ID3D11SamplerState*       linerSampler;
        DirectX::XMFLOAT2         lightUV;
        DirectX::XMMATRIX         lensMatrix;
        RenderParams() : inputSRV(nullptr), linerSampler(nullptr)
        , lightUV(0.0f, 0.0f), lensMatrix(DirectX::XMMatrixIdentity()){
        }
    }; // RenderParams

public:
    PostEffects();
    ~PostEffects();
    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);

    void                      ClearRT(ID3D11DeviceContext*);
    ID3D11Texture2D*          GetTexture() const;
    ID3D11RenderTargetView*   GetRTV() const;
    ID3D11ShaderResourceView* GetSRV() const;

private:
    struct LensFlareBuffer {
        // Row 1: 기본 고스트 제어
        int   count;
        float spacing;
        float threshold;
        float alpha;

        // Row 2: 태양 위치 및 기본 글로우
        DirectX::XMFLOAT2 sunUV;
        float glowSize;
        float starScale;

        // Row 3: 고스트 물리 속성 및 태양 코어
        float ghostPull;
        float ghostIntensity;
        float ghostFalloff;
        float sunCoreTightness;

        // Row 4: 왜곡 및 휘도
        DirectX::XMFLOAT3 distortion;
        float padding1;

        // Row 5: 휘도 기준
        DirectX::XMFLOAT3 luminance;
        float padding2;

        // Row 6: F2 설정 (Offset + Sharpness)
        DirectX::XMFLOAT3 f2Offset;
        float f2Sharpness;

        // Row 7: F2 색상
        DirectX::XMFLOAT3 f2ColorMult;
        float padding3;

        // Row 8: F4 설정 (Offset + Power)
        DirectX::XMFLOAT3 f4Offset;
        float f4Power;

        // Row 9: F4 색상
        DirectX::XMFLOAT3 f4ColorMult;
        float padding4;

        // Row 10: F5 설정 (Offset + Power)
        DirectX::XMFLOAT3 f5Offset;
        float f5Power;

        // Row 11: F5 색상
        DirectX::XMFLOAT3 f5ColorMult;
        float padding5;

        // Row 12: F6 설정 (Offset + Power)
        DirectX::XMFLOAT3 f6Offset;
        float f6Power;

        // Row 13: F6 색상
        DirectX::XMFLOAT3 f6ColorMult;
        float padding6;

        // Row 14~17: 행렬
        DirectX::XMMATRIX lensMatrix;

        LensFlareBuffer()
        {
            // 기본 제어
            count = 8;
            spacing = 0.25f;
            threshold = 0.9f;
            alpha = 1.0f;

            // 태양 관련
            sunUV = { 0.5f, 0.5f };
            glowSize = (float)SharedConstants::ScreenConstants::WIDTH / (float)SharedConstants::ScreenConstants::HEIGHT;
            starScale = 0.8f;

            // 고스트 속성
            ghostPull = 0.1f;
            ghostIntensity = 1.5f;
            ghostFalloff = 1.0f;
            sunCoreTightness = 36.0f;

            distortion = { -0.005f, 0.0f, 0.005f };
            luminance = { 0.3f, 0.59f, 0.11f };

            // F2 파라미터
            f2Offset = { 0.80f, 0.85f, 0.90f };
            f2Sharpness = 32.0f;
            f2ColorMult = { 0.25f, 0.23f, 0.21f };

            // F4 파라미터
            f4Offset = { 0.40f, 0.45f, 0.50f };
            f4Power = 2.4f;
            f4ColorMult = { 6.0f, 5.0f, 3.0f };

            // F5 파라미터
            f5Offset = { 0.20f, 0.40f, 0.60f };
            f5Power = 5.5f;
            f5ColorMult = { 2.0f, 2.0f, 2.0f };

            // F6 파라미터
            f6Offset = { -0.3f, -0.325f, -0.35f };
            f6Power = 1.6f;
            f6ColorMult = { 6.0f, 3.0f, 5.0f };

            // 패딩 초기화
            padding1 = padding2 = padding3 = padding4 = padding5 = padding6 = 0.0f;

            lensMatrix = DirectX::XMMatrixIdentity();
        }
    }; // LensFlareBuffer

private:
    std::unique_ptr<RenderTexture>             m_resRT;
    std::unique_ptr<RenderTexture>             m_tempRT;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_bloomShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_lensFlareShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_lensflareBuffer;
    ID3D11ShaderResourceView*                  m_noiseSRV;

    LensFlareBuffer                            m_lensflareData;
}; // PostEffects