#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include "Resources/ConstantBufferType.h"

class DefaultMesh;
class D3D11State;
class TextureManager;
class Texture;

class SkyBox {
public:
    struct RenderParams {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMFLOAT3 cameraPosition;
        DirectX::XMFLOAT3 lightDir;
        DirectX::XMFLOAT4 lightDiffuse;
    }; // RenderParams

public:
    SkyBox();
    ~SkyBox();

    bool  Init(ID3D11Device*, ID3D11DeviceContext*, HWND, ID3D11SamplerState*, TextureManager*);
    void  Render(ID3D11DeviceContext*, const RenderParams&);
    void  OnGui(); // Imgui 용

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
            groundColor = GROUND_COLOR;
            padding2 = 0.0f;

            groundPrimarySteps = 16;
            groundLightSteps = 4;
            padding3 = { 0.0f, 0.0f };
        }
    }; // AtmosphereBuffer

private:
    bool InitShader(ID3D11Device*, HWND);
    bool UpdateCommonBuffer(ID3D11DeviceContext*, 
        const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, 
        const DirectX::XMFLOAT3&, const DirectX::XMFLOAT3&, const DirectX::XMFLOAT4&);
    bool UpdateAtmosphereBuffer(ID3D11DeviceContext*);
    void GuiAtmosphereBuffer();

private:
    std::unique_ptr<DefaultMesh>               m_cubeMesh;
    // resources
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_commonBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_atmosphereBuffer;

    AtmosphereBuffer                           m_atmosphereData;
    AtmosphereBuffer                           m_prevAtmosphereData;
    ConstantBuffer::CommonBuffer               m_CommonData;
    ConstantBuffer::CommonBuffer               m_prevCommonData;
    ID3D11SamplerState*                        m_sampler;
    float                                      m_renderCount;
}; // SkyBox