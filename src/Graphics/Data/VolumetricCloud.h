#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include <string>
// Utils
#include "SharedConstants/BuffersConstants.h"

class RenderTexture;

class VolumetricCloud {
public:
    struct InitParams {
        ID3D11Device*             device;
        HWND                      hwnd;
        ID3D11ShaderResourceView* cloudMapLUTSRV;
        ID3D11ShaderResourceView* worleyNoiseSRV;
        ID3D11ShaderResourceView* blueNoiseSRV;
        ID3D11SamplerState*       wrapSampler;
        ID3D11SamplerState*       pointSampler;

		InitParams() : device(nullptr), hwnd(0), cloudMapLUTSRV(nullptr),
            worleyNoiseSRV(nullptr), blueNoiseSRV(nullptr),
            wrapSampler(nullptr), pointSampler(nullptr) {
        }
    }; // InitParams

    struct ExecuteParams {
        float                     time;
        ID3D11ShaderResourceView* SkyLUTSRV;
		ID3D11ShaderResourceView* depthSRV;

        ExecuteParams() :
            time(0.0f), SkyLUTSRV(nullptr), depthSRV(nullptr) {
        }
    }; // RenderParams

public:
    VolumetricCloud();
    ~VolumetricCloud();

    bool Init(const InitParams&);
    void Execute(ID3D11DeviceContext*, const ExecuteParams&);
    void OnGui();

    ID3D11ShaderResourceView* GetCloudSRV();

private:
    struct VolumetricCloudBuffer {
        // Row 1
        DirectX::XMFLOAT3 planetCenter;
        float             planetRadius;        // 6371000.0f
        // Row 2
        float             cloudBottom;         // 1350.0f
        float             cloudTop;            // 2350.0f
        float             cloudsLayerBottom;   // -150.0f
        float             cloudsLayerTop;      // -70.0f
        // Row 3
        float             cloudCoverage;       // 0.52f
        float             cloudsLayerCoverage; // 0.41f
		float             cloudBaseScale;      // 1.51f
		float             cloudDetailScale;    // 20.0f
        // Row 4
        float             cloudDensity;        // 0.03f
        float             baseEdgeSoftness;    // 0.1f
        float             bottomSoftness;      // 0.25f
        float             detailStrength;      // 0.225f
        // Row 5
        float             forwardScatteringG;   // 0.8f
        float             backwardScatteringG;  // -0.2f
        float             scatteringLerp;       // 0.5f
        float             minTransmittance;     // 0.1f
        // Row 6
        DirectX::XMFLOAT3 ambientBottom;
        float             padding1;
        // Row 7
        DirectX::XMFLOAT3 ambientTop;
        float             padding2;
        // Row 8
        DirectX::XMFLOAT3 sunsetAmbientBottom;
        float             padding3;
        // Row 9
        DirectX::XMFLOAT3 sunsetAmbientTop;
        float             padding4;
        // Row 10
        DirectX::XMFLOAT3 nightAmbientBottom;
        float             padding5;
        // Row 11
        DirectX::XMFLOAT3 nightAmbientTop;
        float             padding6;
        // Row 12
		DirectX::XMFLOAT2 windDirection;       // (0.5f, 0.5f)
		float             windSpeed;           // 1.0f
		float 		      windScale;
        // Row 13
        float             hgScale;
        float             PowderFactor;
        float             LightingScale;
        float             HorizenFadeScale;

        VolumetricCloudBuffer() {
            using namespace SharedConstants::BuffersConstants;
            planetCenter = { 0.0f, -PLANET_RADIUS, 0.0f };
            planetRadius = PLANET_RADIUS;

            cloudBottom = CLOUD_BOTTOM; 
            cloudTop = CLOUD_TOP;
			cloudsLayerBottom = CLOUDS_LAYER_BOTTOM;
			cloudsLayerTop = CLOUDS_LAYER_TOP;

            cloudCoverage = CLOUD_COVERAGE;
			cloudsLayerCoverage = CLOUDS_LAYER_COVERAGE;
            cloudBaseScale = CLOUD_BASE_SCALE;
			cloudDetailScale = CLOUD_DETAIL_SCALE;

            cloudDensity = CLOUD_DENSITY;
            baseEdgeSoftness = CLOUD_BASE_EDGE_SOFTNESS;
			bottomSoftness = CLOUD_BOTTOM_SOFTNESS;
            detailStrength = CLOUD_DETAIL_STRENGTH;

            forwardScatteringG = CLOUD_FORWARD_SCATTERING_G;
            backwardScatteringG = CLOUD_BACKWARD_SCATTERING_G;
            scatteringLerp = CLOUD_SCATTERING_LERP;
            minTransmittance = CLOUD_MIN_TRANSMITTANCE;

            ambientBottom = CLOUD_DAY_AMBIENT_COLOR_BOTTOM;
			padding1 = 0.0f;
            ambientTop = CLOUD_DAY_AMBIENT_COLOR_TOP;
			padding2 = 0.0f;

            sunsetAmbientBottom = CLOUD_SUNSET_AMBIENT_COLOR_BOTTOM;
            padding3 = 0.0f;
            sunsetAmbientTop = CLOUD_SUNSET_AMBIENT_COLOR_TOP;
            padding4 = 0.0f;

            nightAmbientBottom = CLOUD_NIGHT_AMBIENT_COLOR_BOTTOM;
            padding5 = 0.0f;
            nightAmbientTop = CLOUD_NIGHT_AMBIENT_COLOR_TOP;
            padding6 = 0.0f;

			windDirection = WIND_DIRECTION;
			windSpeed = WIND_SPEED;
            windScale = WIND_SCALE;

            hgScale = HENYEY_GREENSTEIN_SCALE;
            PowderFactor = POWDER_FACTOR;
            LightingScale = LIGHTING_SCALE;
            HorizenFadeScale = HORIZON_FADE_SCALE;
        }
    }; // VolumetricCloudBuffer

private:
    bool UpdateVolumetricCloudBuffer(ID3D11DeviceContext*);

private:
    std::unique_ptr<RenderTexture>              m_resultRT;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_cloudBuffer;
    // buffer
    VolumetricCloudBuffer                       m_cloudBufferData;
    VolumetricCloudBuffer                       m_prevCloudBufferData;
    // srv
    ID3D11SamplerState*                         m_linerWrapSampler;
    ID3D11SamplerState*                         m_pointClampSampler;
    ID3D11ShaderResourceView*                   m_cloudMapLUTSRV;
    ID3D11ShaderResourceView*                   m_worleyNoiseSRV;
    ID3D11ShaderResourceView*                   m_blueNoiseSRV;
}; // VolumetricCloud