#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <wrl/client.h>
#include "Resources/ConstantBufferType.h"
#include "SharedConstants/AtmosphereConstants.h"

class CubeMap;
class D3D11State;

class Atmosphere {
public:
    struct RenderParams {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMFLOAT3 camPos;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT3 lightDir;
    };

    struct CameraBuffer {
        DirectX::XMFLOAT3 cameraPosition;
        float padding;

        DirectX::XMMATRIX viewInv;
        DirectX::XMMATRIX projInv;

        CameraBuffer() :
              cameraPosition(0.0f, 0.0f, 0.0f), padding(0.0f) {
            viewInv = DirectX::XMMatrixIdentity();
            projInv = DirectX::XMMatrixIdentity();
        }
        
	}; // CameraBuffer

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
            using namespace SharedConstants::AtmosphereConstants;

            zenithColor = { 0.0f, 0.2f, 0.6f, 1.0f };
            horizonColor = { 0.81f, 0.38f, 0.66f, 1.0f };

            planetCenter = { 0.0f, -PLANET_RADIUS, 0.0f };
            planetRadius = PLANET_RADIUS;
            atmoRadius = ATMOSPHERE_RADIUS;
			padding1 = { 0.0f, 0.0f, 0.0f };

            rayleighBeta = RAYLEIGH_SCATTERING_COEFFICIENT;
            mieBeta = MIE_BETA;
            absorptionBeta = MIE_SCATTERING_COEFFICIENT;
            ambientBeta = 0.0f;

            rayleighHeight =RAYLEIGH_HEIGHT;
            mieHeight = MIE_HEIGHT;
            absorptionHeight = ABSORPTION_HEIGHT;
            absorptionFalloff = ABSORPTION_FALLOFF;
            g = 0.9f;
            primarySteps = 32;
            lightSteps = 8;
            intensity = 40.0f;
			groundColor = { 0.0f, 0.25f, 0.05f };
			padding2 = 0.0f;

            groundPrimarySteps = 16;
			groundLightSteps = 4;
			padding3 = { 0.0f, 0.0f };
        }
    }; // AtmosphereBuffer

    struct BlendBuffer {
        float blendFactor;
        DirectX::XMFLOAT3 padding;

        BlendBuffer() {
            blendFactor = 1.0f;
            padding = { 0.0f, 0.0f, 0.0f };
        }
    }; // BlendBuffer

public:
    Atmosphere();
    ~Atmosphere();

    bool Init(ID3D11Device*, ID3D11DeviceContext*, HWND);
    void Bake(ID3D11DeviceContext*, D3D11State*, const RenderParams&);
    void Update(float);

    ID3D11ShaderResourceView* GetCubeMapSRV(int) const;
    float                     GetBlendFactor() const;
	int                       GetActiveIndex() const;
	int                       GetTargetIndex() const;
	bool					  IsInterpolating() const;
    AtmosphereBuffer&         GetAtmosphereBuffer();

private:
	bool InitShader(ID3D11Device*, HWND);
	bool UpdateMatrixBuffer(ID3D11DeviceContext*, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&);
	bool UpdateLightBuffer(ID3D11DeviceContext*, const DirectX::XMFLOAT4&, const DirectX::XMFLOAT3&);
	bool UpdateAtmosphereBuffer(ID3D11DeviceContext*);
	bool UpdateCameraBuffer(ID3D11DeviceContext*, const DirectX::XMFLOAT3&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&);
    void PrepareFaceRender(ID3D11DeviceContext*, int, const RenderParams&);

private:
	// Resources
    std::unique_ptr<CubeMap>                   m_cubeMaps[2];
	// Shader resources
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_matrixBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_lightBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cameraBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_atmosphereBuffer;
    // etc
    ConstantBuffer::MatrixBuffer               m_prevMatrixData;
    ConstantBuffer::LightBuffer                m_prevLightData;
	CameraBuffer                               m_prevCameraData;
    AtmosphereBuffer                           m_atmosphereData;
    AtmosphereBuffer                           m_prevAtmosphereData;
    DirectX::XMFLOAT4                          m_zenithColor;
    DirectX::XMFLOAT4                          m_horizonColor;
    int                                        m_activeIdx;
    int                                        m_targetIdx;
    float                                      m_blendFactor;
    bool                                       m_isInterpolating;
}; // Atmosphere