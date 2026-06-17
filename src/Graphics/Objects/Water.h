#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include <string>
// Utils
#include "SharedConstants/BuffersConstants.h"

class DefaultMesh;

class Water {
public:
    struct InitParams {
        ID3D11Device*             device;
        HWND                      hwnd;
        float                     waterHeight;    // 물의 기준 높이
        float                     waterScale;     // 물 평면의 전체 크기
        int                       gridDensity;    // 정점 분할 수 (파도 애니메이션용)
        ID3D11ShaderResourceView* waterNormalSRV;
        ID3D11ShaderResourceView* waterWaveNormalSRV;
        ID3D11ShaderResourceView* flowSRV;
        ID3D11SamplerState*       linearWrapSampler;

        InitParams() : device(nullptr), hwnd(nullptr),
            waterHeight(0.0f), waterScale(2000.0f), gridDensity(256),
            waterNormalSRV(nullptr), waterWaveNormalSRV(nullptr), flowSRV(nullptr),
            linearWrapSampler(nullptr) {
        }
    }; // InitParams

    struct RenderParams {
        ID3D11ShaderResourceView* reflectionSRV;  // 반사 RTT
        ID3D11ShaderResourceView* refractionSRV;  // 굴절 RTT
        ID3D11ShaderResourceView* sceneDepthSRV;   // Beer's Law용 메인 씬 깊이 텍스처

        RenderParams() : reflectionSRV(nullptr), refractionSRV(nullptr),
            sceneDepthSRV(nullptr) {
        }
    }; // RenderParams

public:
    Water();
    ~Water();

    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);
    float GetWaterHeight();

private:
    struct WaterBuffer {
        DirectX::XMMATRIX worldMatrix;

        float             waterHeight;
        DirectX::XMFLOAT3 padding1;

        DirectX::XMFLOAT3 waterColorShallow;
        float             padding2;

        DirectX::XMFLOAT3 waterColorDepp;
        float             padding3;

        float             distortion;
        float             reflectivity;
        float             density;
        float             padding4;

        WaterBuffer() {
            worldMatrix = DirectX::XMMatrixIdentity();
            waterHeight = 0.0f;
            padding1 = { 0.0f, 0.0f, 0.0f };
            waterColorShallow = SharedConstants::BuffersConstants::WATER_COLOR_SHALLOW;
            padding2 = 0.0f;
            waterColorDepp = SharedConstants::BuffersConstants::WATER_COLOR_DEEP;
            padding3 = 0.0f;
            distortion = SharedConstants::BuffersConstants::WATER_DISTORTION;
            reflectivity = SharedConstants::BuffersConstants::WATER_REFLECTIVITY;
            density = SharedConstants::BuffersConstants::WATER_DENSITY;
            padding4 = 0.0f;
        }
    }; // WaterBuffer

private:
    bool InitShader(ID3D11Device*, HWND);

private:
    std::unique_ptr<DefaultMesh>               m_waterMesh;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_waterBuffer;

    WaterBuffer                                m_waterBufferData;
    float                                      m_waterHeight;
    ID3D11ShaderResourceView*                  m_waterNormalSRV;
    ID3D11ShaderResourceView*                  m_waterWaveNormalSRV;
    ID3D11ShaderResourceView*                  m_flowSRV;
    ID3D11SamplerState*                        m_linearSampler;
}; // Water