#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include <string>
// Utils
#include "SharedConstants/BuffersConstants.h"

class RenderTexture;

class VolumetricFog {
public:
    struct InitParams {
        ID3D11Device* device;
        HWND                      hwnd;
        ID3D11ShaderResourceView* heightMapSRV;
        ID3D11ShaderResourceView* worleyNoiseSRV;
        ID3D11SamplerState*       wrapSampler;
        ID3D11SamplerState*       pointSampler;
        int                       screenWidth;
        int                       screenHeight;

        InitParams() : device(nullptr), hwnd(nullptr),
            heightMapSRV(nullptr), worleyNoiseSRV(nullptr),
            wrapSampler(nullptr), pointSampler(nullptr),
            screenWidth(0), screenHeight(0) {
        }
    }; // InitParams

    struct ExecuteParams {
        float                     time;
        ID3D11ShaderResourceView* depthSRV;
        ID3D11ShaderResourceView* normalSRV;
        float                     terrainWidth;
        float                     terrainDepth;
        float                     terrainHeightScale;


        ExecuteParams() : time(0.0f), depthSRV(nullptr), normalSRV(nullptr),
            terrainWidth(0.0f), terrainDepth(0.0f), terrainHeightScale(SharedConstants::BuffersConstants::HEIGHT_SCALE) {
        }
    }; // ExecuteParams

public:
    VolumetricFog();
    ~VolumetricFog();

    bool Init(const InitParams&);
    void Execute(ID3D11DeviceContext*, const ExecuteParams&);
    void OnGui();

    ID3D11ShaderResourceView* GetFogSRV();

private:
    struct VolumetricFogBuffer {
        // Row 1: 높이 기반 밀도
        float fogBaseHeight;
        float fogHeightFalloff;
        float fogDensity;
        float fogMaxDistance;
        // Row 2: 노이즈
        float fogNoiseScale;
        float fogNoiseStrength;
        float fogWindSpeed;
        float fogPhaseG;
        // Row 3: 색상
        DirectX::XMFLOAT3  fogColor;
        float              fogAmbientStrength;
        // Row 4: 경사(노말) 감쇠 + 바람 방향
        float              fogSlopeMin;
        float              fogSlopeMax;
        DirectX::XMFLOAT2  fogWindDirection;
        // Row 5: 지형 월드 매핑
        float              terrainWidth;
        float              terrainDepth;
        float              terrainHeightScale;
        int                fogMarchSteps;
        //
        DirectX::XMFLOAT4  padding1;

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

            fogSlopeMin = FOG_SLOPE_MIN;
            fogSlopeMax = FOG_SLOPE_MAX;
            fogWindDirection = FOG_WIND_DIRECTION;

            terrainWidth = 0.0f;
            terrainDepth = 0.0f;
            terrainHeightScale = HEIGHT_SCALE;
            fogMarchSteps = FOG_MARCH_STEPS;

            padding1 = { 0.0f, 0.0f, 0.0f, 0.0f };
        }
    }; // VolumetricFogBuffer

private:
    bool UpdateVolumetricFogBuffer(ID3D11DeviceContext*);

private:
    std::unique_ptr<RenderTexture>              m_resultRT;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_fogBuffer;
    // buffer
    VolumetricFogBuffer                         m_fogBufferData;
    VolumetricFogBuffer                         m_prevFogBufferData;
    // srv
    ID3D11SamplerState*                         m_linerWrapSampler;
    ID3D11SamplerState*                         m_pointClampSampler;
    ID3D11ShaderResourceView*                   m_heightMapSRV;
    ID3D11ShaderResourceView*                   m_worleyNoiseSRV;
}; // VolumetricFog