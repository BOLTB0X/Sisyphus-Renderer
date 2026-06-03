#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include "Resources/ConstantBufferType.h"

class RenderTexture;

class AtmosphereMap {
public:
    struct InitParams {
        ID3D11Device*             device;
        ID3D11DeviceContext*      context;
        HWND                      hwnd;
        ID3D11SamplerState*       linerWrapSampler;

        InitParams() : device(nullptr), context(nullptr), hwnd(0),
            linerWrapSampler(nullptr) {
        }
    }; // InitParams

    struct ExecuteParams {
        DirectX::XMFLOAT3 LightDirection;
        DirectX::XMFLOAT3 CameraPosition;
        float             time;
        ExecuteParams() : LightDirection(0.0f, 1.0f, 0.0f), CameraPosition(0.0f, 0.0f, 0.0f), time(0.0f) {
        }
	}; // ExecuteParams

public:
    AtmosphereMap();
    ~AtmosphereMap();

    bool Init(const InitParams&);
    void Execute(ID3D11DeviceContext*, const ExecuteParams&);
    void OnGui();

    ID3D11ShaderResourceView* GetLUT();

private:
    struct AtmosphereBuffer {
        // Row 1
        DirectX::XMFLOAT4 zenithColor;
        // Row 2
        DirectX::XMFLOAT4 horizonColor;
        // Row 3
        DirectX::XMFLOAT3 planetCenter;
        float             planetRadius;
        // Row 4
        float             atmoRadius;
        DirectX::XMFLOAT3 padding1;
        // Row 5
        DirectX::XMFLOAT3 rayleighBeta;
        float             mieBeta;
        // Row 6
        DirectX::XMFLOAT3 absorptionBeta;
        float             ambientBeta;
        // Row 7
        float             rayleighHeight;
        float             mieHeight;
        float             absorptionHeight;
        float             absorptionFalloff;
        // Row 8
        float             g;
        int               primarySteps;
        int               lightSteps;
        float             intensity;
        // Row 9
        DirectX::XMFLOAT3 groundColor;
        float             padding2;
        // Row 10
        int               groundPrimarySteps;
        int               groundLightSteps;
        DirectX::XMFLOAT2 padding3;

        AtmosphereBuffer() {
            using namespace SharedConstants::BuffersConstants;

            zenithColor = ZENITH_COLOR;
            horizonColor = HORIZON_COLOR;

            planetCenter = { 0.0f, -PLANET_RADIUS, 0.0f };
            planetRadius = PLANET_RADIUS;
            atmoRadius = ATMOSPHERE_RADIUS;
            padding1 = { 0.0f, 0.0f, 0.0f };

            rayleighBeta = RAYLEIGH_SCATTERING_COEFFICIENT;
            mieBeta = MIE_BETA;
            absorptionBeta = MIE_SCATTERING_COEFFICIENT;
            ambientBeta = 0.0f;

            rayleighHeight = RAYLEIGH_HEIGHT;
            mieHeight = MIE_HEIGHT;
            absorptionHeight = ABSORPTION_HEIGHT;
            absorptionFalloff = ABSORPTION_FALLOFF;
            g = G;
            primarySteps = 32;
            lightSteps = 8;
            intensity = ATMOSPHERE_INTENWSITY;
            groundColor = DARK_SAND;
            padding2 = 0.0f;

            groundPrimarySteps = 16;
            groundLightSteps = 4;
            padding3 = { 0.0f, 0.0f };
        }
    }; // AtmosphereBuffer

private:
    void Compute(ID3D11DeviceContext*);
    bool UpdateAtmosphereBuffer(ID3D11DeviceContext*);
    bool UpdateResolutionBuffer(ID3D11DeviceContext*);

private:
    std::unique_ptr<RenderTexture>              m_LUT;
    // shader resources
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_atmosphereBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_resolutionBuffer;
    ID3D11SamplerState*                         m_linerWrapSampler;
    // etc
    AtmosphereBuffer                            m_atmosphereData;
    ConstantBuffer::ResolutionBuffer            m_resolutionData;
    AtmosphereBuffer                            m_prevAtmosphereData;
    ConstantBuffer::ResolutionBuffer            m_prevResolutionData;
    DirectX::XMFLOAT3                           m_preLightDirection;
    DirectX::XMFLOAT3                           m_preCameraPosition;
}; // AtmosphereMap