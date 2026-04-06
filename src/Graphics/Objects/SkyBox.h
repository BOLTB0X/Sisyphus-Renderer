#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include "Resources/ConstantBufferType.h"
#include "Resources/CloudMap.h"

class DefaultMesh;
class D3D11State;
class VolumeTexture;
class RenderTexture;

class SkyBox {
public:
    struct InitParams {
        ID3D11Device*                  device;
        ID3D11DeviceContext*           context;
        HWND                           hwnd;
        std::shared_ptr<VolumeTexture> noiseTexture;
        ID3D11SamplerState*            sampler;
        ID3D11ShaderResourceView*      depth;
    }; // InitParams

    struct RenderParams {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMFLOAT3 cameraPosition;
        DirectX::XMFLOAT3 lightDir;
        DirectX::XMFLOAT4 lightDiffuse;
        float             time;
    }; // RenderParams

public:
    SkyBox();
    ~SkyBox();

    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void OnGui(); // Imgui 용

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

    struct CloudBuffer {
        float cloudMinHeight;
        float cloudMaxHeight;
        DirectX::XMFLOAT2 padding;

        CloudBuffer() {
            using namespace SharedConstants::BuffersConstants;
            cloudMinHeight = CLOUD_MIN_HEIGHT;
            cloudMaxHeight = CLOUD_MAX_HEIGHT;
            padding = { 0.0f, 0.0f };
        }
    }; // CloudBuffer

private:
    bool InitShader(ID3D11Device*, HWND);
    bool UpdateCommonBuffer(ID3D11DeviceContext*, 
        const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, 
        const DirectX::XMFLOAT3&, const DirectX::XMFLOAT3&, const DirectX::XMFLOAT4&);
    bool UpdateAtmosphereBuffer(ID3D11DeviceContext*);
    bool UpdateCloudBuffer(ID3D11DeviceContext*);
    void GuiAtmosphere();
    void GuiWeatherMap();

private:
    std::unique_ptr<DefaultMesh>               m_CubeMesh;
    std::unique_ptr<CloudMap>                  m_CloudMap;
    // shader resources
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_commonBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_atmosphereBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cloudBuffer;
    // buffers
    AtmosphereBuffer                           m_atmosphereData;
    AtmosphereBuffer                           m_prevAtmosphereData;
    CloudBuffer                                m_cloudData;
    CloudBuffer                                m_prevCloudData;
    ConstantBuffer::CommonBuffer               m_CommonData;
    ConstantBuffer::CommonBuffer               m_prevCommonData;
    CloudMap::CloudMapBuffer                   m_cloudMapData;
    // textures
    std::shared_ptr<VolumeTexture>             m_noise;
    std::unique_ptr<RenderTexture>             m_weatherMapRT;
    ID3D11SamplerState*                        m_linerWrapSampler;
    ID3D11ShaderResourceView*                  m_depthSRV;
}; // SkyBox