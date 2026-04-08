#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include "Utils/SharedConstants/BuffersConstants.h"

class DefaultMesh;
class D3D11State;
class VolumeTexture;
class RenderTexture;
class Composite;

class SkyBox {
public:
    struct InitParams {
        ID3D11Device*                  device;
        ID3D11DeviceContext*           context;
        HWND                           hwnd;
        ID3D11SamplerState*            sampler;
        ID3D11ShaderResourceView*      depth;
        ID3D11ShaderResourceView*      weather;
        ID3D11ShaderResourceView*      baseNoise;
        ID3D11ShaderResourceView*      detailNoise;

        InitParams() : device(nullptr), context(nullptr), hwnd(nullptr),
            weather(nullptr), baseNoise(nullptr), detailNoise(nullptr),
            sampler(nullptr), depth(nullptr) {
		}
    }; // InitParams

    struct RenderParams {
        DirectX::XMFLOAT3 camPos;
        float             time;
		RenderParams() : camPos(0.0f, 0.0f, 0.0f), time(0.0f) {
        }
    }; // RenderParams

public:
    SkyBox();
    ~SkyBox();

    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void OnGui(); // Imgui 용

private:
    struct WolrdBuffer {
        DirectX::XMMATRIX world;
        
        WolrdBuffer() {
            world = DirectX::XMMatrixIdentity();
		}
	}; // WorldBuffer;

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

    struct CloudBoxBuffer {
        DirectX::XMFLOAT4 boxCenter;    // 박스 중심 좌표
        DirectX::XMFLOAT4 boxSize;      // 박스 크기 (예: 40000.0f, 5000.0f, 40000.0f)
        float earthRadius;              // 6371000.0f (미터 단위)
        float cloudMinHeight;           // 1500.0f (구름 시작 고도)
        float cloudMaxHeight;           // 4000.0f (구름 끝 고도)
        float padding;

        CloudBoxBuffer() {
            using namespace SharedConstants::BuffersConstants;
            // 1. 지구 반지름 (미터)
            //earthRadius = PLANET_RADIUS;
            earthRadius = 6371000.0f;

            // 2. 구름 고도 설정 (현실적인 적운/층적운 고도)
            cloudMinHeight = 1500.0f; // 1.5km 상공에서 구름 시작
            cloudMaxHeight = 4500.0f; // 4.5km 상공에서 구름 끝 (두께 3km)

            // 3. 박스 크기
            // Y축 크기는 구름의 두께(Max - Min)와 정확히 일치해야 합니다.
            // X, Z축은 카메라를 덮을 만큼 거대해야 합니다 (사방으로 100km = 200,000m)
            //boxSize = DirectX::XMFLOAT4(200000.0f, cloudMaxHeight - cloudMinHeight, 200000.0f, 0.0f);
            boxSize = DirectX::XMFLOAT4(500.0f, cloudMaxHeight - cloudMinHeight, 500.0f, 0.0f);

            // Center 초기화 (업데이트에서 덮어씌움)
            boxCenter = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			padding = 0.0f;
        }
    }; // CloudBoxBuffer

private:
    bool InitShader(ID3D11Device*, HWND);
    bool UpdateAtmosphereBuffer(ID3D11DeviceContext*);
    bool UpdateCloudBoxBuffer(ID3D11DeviceContext*, const DirectX::XMFLOAT3&);
    void GuiAtmosphere();

private:
    std::unique_ptr<DefaultMesh>               m_CubeMesh;
	std::unique_ptr<Composite>                 m_Composite;
    // shader resources
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_compositeVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_compositePS;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_worldBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_atmosphereBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cloudBoxBuffer;
    // buffers
    AtmosphereBuffer                           m_atmosphereData;
    AtmosphereBuffer                           m_prevAtmosphereData;
    CloudBoxBuffer                             m_cloudBoxData;
    CloudBoxBuffer                             m_prevCloudBoxData;
    WolrdBuffer                                m_WolrdData;
    // textures
    std::unique_ptr<RenderTexture>             m_volumetricRT;
    ID3D11SamplerState*                        m_linerWrapSampler;
    ID3D11ShaderResourceView*                  m_depthSRV;
    ID3D11ShaderResourceView*                  m_weatherSRV;
    ID3D11ShaderResourceView*                  m_baseNoiseSRV;
    ID3D11ShaderResourceView*                  m_detailNoiseSRV;

}; // SkyBox