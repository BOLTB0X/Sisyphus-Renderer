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
        ID3D11ShaderResourceView* CloudMapLUTSRV;
        ID3D11ShaderResourceView* worleyNoiseSRV;
        ID3D11ShaderResourceView* blueNoiseSRV;
        ID3D11SamplerState*       sampler;

		InitParams() : device(nullptr), hwnd(0), CloudMapLUTSRV(nullptr),
            worleyNoiseSRV(nullptr), blueNoiseSRV(nullptr), sampler(nullptr) {
        }
    }; // InitParams

    struct ExecuteParams {
        float                     time;
        ID3D11ShaderResourceView* SkyLUTSRV;

        ExecuteParams() :
            time(0.0f), SkyLUTSRV(nullptr) {
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
        // Row 1: 행성 기본 정보 (미터 단위)
        DirectX::XMFLOAT3 planetCenter;
        float             planetRadius;      // 6,371,000.0f

        // Row 2: 대기 및 구름 고도 영역
        float             atmoRadius;        // 6,471,000.0f
        float             cloudBottom;    // 지표면으로부터 최소 높이 (예: 1,500m)
        float             cloudTop;    // 지표면으로부터 최대 높이 (예: 4,000m)
        float             padding1;

        // Row 3: 구름 밀도 및 형상 제어
        float             cloudCoverage;     // CLOUDS_COVERAGE (0.52f)
        float             cloudDensity;      // CLOUDS_DENSITY (0.03f)
        float             baseEdgeSoftness;  // CLOUDS_BASE_EDGE_SOFTNESS (0.1f)
        float             detailStrength;    // CLOUDS_DETAIL_STRENGTH (0.225f)

        // Row 4: 빛의 산란 관련 (Henyey-Greenstein)
        float             forwardScatteringG;  // 0.8f
        float             backwardScatteringG; // -0.2f
        float             scatteringLerp;      // 0.5f
        float             minTransmittance;    // 0.1f

        // Row 5: 구름 색상 (Ambient)
        DirectX::XMFLOAT3 ambientTop;          // CLOUDS_AMBIENT_COLOR_TOP
        float             padding2;

        // Row 6: 구름 색상 (Ambient)
        DirectX::XMFLOAT3 ambientBottom;       // CLOUDS_AMBIENT_COLOR_BOTTOM
        float             padding3;

        VolumetricCloudBuffer() {
            using namespace SharedConstants::BuffersConstants;
            planetCenter = { 0.0f, -PLANET_RADIUS * 0.001f, 0.0f };
            planetRadius = PLANET_RADIUS * 0.001f;
            atmoRadius = ATMOSPHERE_RADIUS * 0.001f;

            cloudBottom = CLOUD_BOTTOM * 0.001f; 
            cloudTop = CLOUD_TOP * 0.001f;
			padding1 = 0.0f;

            cloudCoverage = CLOUD_COVERAGE;
            cloudDensity = CLOUD_DENSITY;
            baseEdgeSoftness = CLOUD_BASE_EDGE_SOFTNESS;
            detailStrength = CLOUD_DETAIL_STRENGTH;

            forwardScatteringG = CLOUD_FORWARD_SCATTERING_G;
            backwardScatteringG = CLOUD_BACKWARD_SCATTERING_G;
            scatteringLerp = CLOUD_SCATTERING_LERP;
            minTransmittance = CLOUD_MIN_TRANSMITTANCE;

            ambientTop = CLOUD_AMBIENT_COLOR_TOP;
			padding2 = 0.0f;
            ambientBottom = CLOUD_AMBIENT_COLOR_BOTTOM;
			padding3 = 0.0f;
        }
    }; // VolumetricCloudBuffer

private:
    bool UpdateVolumetricCloudBuffer(ID3D11DeviceContext*);
    bool UpdateResolutionBuffer(ID3D11DeviceContext*);

private:
    std::unique_ptr<RenderTexture>              m_resultRT;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_cloudBuffer;
    // buffer
    VolumetricCloudBuffer                       m_cloudBufferData;
    VolumetricCloudBuffer                       m_prevCloudBufferData;
    // srv
    ID3D11SamplerState*                         m_linerWrapSampler;
    ID3D11ShaderResourceView*                   m_cloudMapLUTSRV;
    ID3D11ShaderResourceView*                   m_worleyNoiseSRV;
    ID3D11ShaderResourceView*                   m_blueNoiseSRV;
}; // VolumetricCloud