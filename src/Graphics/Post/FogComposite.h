#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include <string>
// Utils
#include "SharedConstants/BuffersConstants.h"

class RenderTexture;

class FogComposite {
public:
    struct InitParams {
        ID3D11Device*             device;
        HWND                      hwnd;
        ID3D11ShaderResourceView* heightMapSRV;
        ID3D11ShaderResourceView* noiseMapSRV;
        ID3D11ShaderResourceView* worleyNoiseSRV;
        ID3D11SamplerState*       wrapSampler;
        ID3D11SamplerState*       pointSampler;
        int                       screenWidth;
        int                       screenHeight;

        InitParams() : device(nullptr), hwnd(nullptr),
            heightMapSRV(nullptr), noiseMapSRV(nullptr), worleyNoiseSRV(nullptr),
            wrapSampler(nullptr), pointSampler(nullptr),
            screenWidth(0), screenHeight(0) {
        }
    }; // InitParams

    struct RenderParams {
        ID3D11ShaderResourceView* sceneSRV;
        ID3D11ShaderResourceView* depthSRV;
        ID3D11ShaderResourceView* normalSRV;
        float                     terrainWidth;
        float                     terrainDepth;
        float                     terrainHeightScale;

        RenderParams() : sceneSRV(nullptr), depthSRV(nullptr), normalSRV(nullptr),
            terrainWidth(0.0f), terrainDepth(0.0f), terrainHeightScale(0.0f) {
        }
    }; // RenderParams

public:
    FogComposite();
    ~FogComposite();

    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);

	void                      OnGui();
    void                      ClearRT(ID3D11DeviceContext*);
    ID3D11Texture2D*          GetTexture() const;
    ID3D11RenderTargetView*   GetRTV() const;
    ID3D11ShaderResourceView* GetSRV() const;

private:
    struct VolumetricFogBuffer {
        // Row 1: 높이 기반 밀도
        float              fogBaseHeight;
        float              fogHeightFalloff;
        float              fogDensity;
        float              fogMaxDistance;
        // Row 2: 노이즈
        float              fogNoiseScale;
        float              fogNoiseStrength;
        float              fogWindSpeed;
        float              padding1;
        // Row 3: 색상
        DirectX::XMFLOAT3  fogColor;
        float              fogAmbientStrength;
        // Row 4: 경사(노말) 감쇠 + 바람 방향
        DirectX::XMFLOAT2  fogWindDirection;
        DirectX::XMFLOAT2  padding2;
        // Row 5: 지형 월드 매핑
        float              terrainWidth;
        float              terrainDepth;
        float              terrainHeightScale;
        int                fogMarchSteps;
        //
        DirectX::XMFLOAT4  padding3;

        VolumetricFogBuffer() {
            using namespace SharedConstants::BuffersConstants;
            fogBaseHeight = FOG_BASE_HEIGHT;
            fogHeightFalloff = FOG_HEIGHT_FALLOFF;
            fogDensity = FOG_DENSITY;
            fogMaxDistance = FOG_MAX_DISTANCE;

            fogNoiseScale = FOG_NOISE_SCALE;
            fogNoiseStrength = FOG_NOISE_STRENGTH;
            fogWindSpeed = FOG_WIND_SPEED;
            padding1 = 0.0f;

            fogColor = FOG_COLOR;
            fogAmbientStrength = FOG_AMBIENT_STRENGTH;

            fogWindDirection = FOG_WIND_DIRECTION;
			padding2 = { 0.0f, 0.0f };

            terrainWidth = 0.0f;
            terrainDepth = 0.0f;
            terrainHeightScale = HEIGHT_SCALE;
            fogMarchSteps = FOG_MARCH_STEPS;

            padding3 = { 0.0f, 0.0f, 0.0f, 0.0f };
        }
    }; // VolumetricFogBuffer

private:
    bool InitShader(ID3D11Device*, HWND);
    bool UpdateVolumetricFogBuffer(ID3D11DeviceContext*);

private:
    std::unique_ptr<RenderTexture>             m_compositeRT;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_fogBuffer;
    // buffer
    VolumetricFogBuffer                        m_fogBufferData;
    VolumetricFogBuffer                        m_prevFogBufferData;
    // srv
    ID3D11SamplerState*                        m_linerWrapSampler;
    ID3D11SamplerState*                        m_pointClampSampler;
    ID3D11ShaderResourceView*                  m_heightMapSRV;
    ID3D11ShaderResourceView*                  m_noiseMapSRV;
    ID3D11ShaderResourceView*                  m_worleyNoiseSRV;
}; // FogComposite