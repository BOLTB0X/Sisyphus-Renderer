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
		DirectX::XMFLOAT3         cameraPosition;
        ID3D11ShaderResourceView* sceneSRV;
        ID3D11ShaderResourceView* depthSRV;
        ID3D11ShaderResourceView* normalSRV;
        float                     terrainWidth;
        float                     terrainDepth;
        float                     terrainHeightScale;

        RenderParams() : cameraPosition(0.0f, 0.0f, 0.0f), sceneSRV(nullptr), depthSRV(nullptr), normalSRV(nullptr),
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
        // Row 1
        float             fogBaseHeight;
        float             fogHeightFalloff;
        float             fogDensity;
        float             fogMaxDistance;
        // Row 2
        float             fogNoiseScale;
        float             fogNoiseStrength;
        float             fogWindSpeed;
        float             fogPhaseG;
        // Row 3
        DirectX::XMFLOAT3 fogColor;
        float             fogAmbientStrength;
        // Row 4
        DirectX::XMFLOAT2 fogWindDirection;
        float             fogLightMarchDist;
        int               fogLightMarchSteps;
        // Row 5: 가두는 구
        DirectX::XMFLOAT3 fogSphereCenter;
        float             fogSphereRadius;
        // Row 6
        float             fogEdgeSoftness;
        int               fogMarchSteps;
        DirectX::XMFLOAT2 padding1;

        VolumetricFogBuffer() {
            using namespace SharedConstants::BuffersConstants;
            fogBaseHeight = FOG_BASE_HEIGHT;
            fogHeightFalloff = FOG_HEIGHT_FALLOFF;
            fogDensity = FOG_DENSITY;
            fogMaxDistance = FOG_MAX_DISTANCE;

            fogNoiseScale = FOG_NOISE_SCALE;
            fogNoiseStrength = FOG_NOISE_STRENGTH;
            fogWindSpeed = FOG_WIND_SPEED;
            fogPhaseG = FOG_PHASE_G;

            fogColor = FOG_COLOR;
            fogAmbientStrength = FOG_AMBIENT_STRENGTH;

            fogWindDirection = FOG_WIND_DIRECTION;
            fogLightMarchDist = FOG_LIGHT_MARCH_DIST;
            fogLightMarchSteps = FOG_LIGHT_MARCH_STEPS;

            fogSphereCenter = { 0.0f, 0.0f, 0.0f };
            fogSphereRadius = FOG_SPHERE_RADIUS;

            fogEdgeSoftness = FOG_EDGE_SOFTNESS;
            fogMarchSteps = FOG_MARCH_STEPS;
            padding1 = { 0.0f, 0.0f };
        }
    }; // VolumetricFogBuffer

    struct DeferredFogBuffer {
        // Row 1: 거리 기반
        float             startDistance;
        float             density;
        float             maxDistance;
        float             baseHeight;
        // Row 2: 높이 기반
        float             heightFalloff;
        float             noiseScale;
        float             noiseStrength;
        float             windSpeed;
        // Row 3
        DirectX::XMFLOAT3 color;
        float             padding; // 사용 안 함, 정렬용
        // Row 4
        DirectX::XMFLOAT2 windDirection;
        float             slopeMin;
        float             slopeMax;

        DeferredFogBuffer() {
            using namespace SharedConstants::BuffersConstants;
            startDistance = 500.0f;
            density = 0.0015f;
            maxDistance = 8000.0f;
            baseHeight = 0.0f;

            heightFalloff = 0.01f;
            noiseScale = 0.001f;
            noiseStrength = 0.5f;
            windSpeed = 0.05f;

            color = { 0.75f, 0.8f, 0.85f };
            padding = 0.0f;

            windDirection = FOG_WIND_DIRECTION;
            slopeMin = 0.3f;
            slopeMax = 1.0f;
        }
    }; // DeferredFogBuffer

private:
    bool InitShader(ID3D11Device*, HWND);
    bool UpdateVolumetricFogBuffer(ID3D11DeviceContext*);
	void GuiVolumetricFogBuffer();
	void GuiDeferredFogBuffer();

private:
    std::unique_ptr<RenderTexture>             m_compositeRT;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_fogBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_deferredFogBuffer;
    // buffer
    VolumetricFogBuffer                        m_fogBufferData;
    VolumetricFogBuffer                        m_prevFogBufferData;

	DeferredFogBuffer                          m_deferredFogBufferData;
	DeferredFogBuffer                          m_prevDeferredFogBufferData;
    // srv
    ID3D11SamplerState*                        m_linerWrapSampler;
    ID3D11SamplerState*                        m_pointClampSampler;
    ID3D11ShaderResourceView*                  m_heightMapSRV;
    ID3D11ShaderResourceView*                  m_noiseMapSRV;
    ID3D11ShaderResourceView*                  m_worleyNoiseSRV;
}; // FogComposite