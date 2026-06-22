#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
// Components
#include "Components/Transform.h"
// Resource
#include "Resources/ConstantBuffer.h"
// Utils
#include "SharedConstants/BuffersConstants.h"

class DefaultMesh;

class Water {
public:
    struct InitParams {
        ID3D11Device*             device;
        HWND                      hwnd;
        float                     waterHeight;
        int                       screenWidth;
        int		                  screenHeight;
        ID3D11ShaderResourceView* waterNormalSRV;
        ID3D11ShaderResourceView* waterWaveNormalSRV;
        ID3D11ShaderResourceView* flowSRV;
        ID3D11SamplerState*       linearWrapSampler;

        InitParams() : device(nullptr), hwnd(nullptr),
            waterHeight(0.0f), screenWidth(0), screenHeight(0),
            waterNormalSRV(nullptr), waterWaveNormalSRV(nullptr), flowSRV(nullptr),
            linearWrapSampler(nullptr) {
        }
    }; // InitParams

    struct RenderParams {
        DirectX::XMFLOAT3         cameraPosition;
        DirectX::XMMATRIX         world;
        ID3D11ShaderResourceView* sceneSRV;
        ID3D11ShaderResourceView* reflectionSRV;
        ID3D11ShaderResourceView* sceneDepthSRV;

        RenderParams() : world(DirectX::XMMatrixIdentity()),
            sceneSRV(nullptr), reflectionSRV(nullptr),
            sceneDepthSRV(nullptr) {
            cameraPosition = { 0.0f, 0.0f, 0.0f };
        }
    }; // RenderParams

public:
    Water();
    ~Water();

    bool Init(const InitParams&);

    void Render(ID3D11DeviceContext*, const RenderParams&);
    void OnGui();

    void              SetReflectView(DirectX::XMMATRIX);
    float             GetWaterHeight() const;
    DirectX::XMMATRIX GetWorldMatrix();

private:
    struct WaterBuffer {
        float             waterHeight;  // 추가
        DirectX::XMFLOAT3 padding0;

        DirectX::XMFLOAT3 waterColorShallow;
        float             padding1;

        DirectX::XMFLOAT3 waterColorDeep;
        float             padding2;

        float             waterDistortion;
        float             reflectivity;
        float             density;
        float             wPadding3;

        WaterBuffer()
            : waterHeight(0.0f), padding0(0.0f, 0.0f, 0.0f),
            waterColorShallow(SharedConstants::BuffersConstants::WATER_COLOR_SHALLOW), padding1(0.0f),
            waterColorDeep(SharedConstants::BuffersConstants::WATER_COLOR_DEEP), padding2(0.0f),
            waterDistortion(0.01f), reflectivity(0.2f),
            density(0.4f), wPadding3(0.0f) {
        }
    }; // WaterBuffer

private:
    bool InitShader(ID3D11Device*, HWND, const int&, const int&);

private:
    std::unique_ptr<DefaultMesh>               m_waterMesh;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;

    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_waterBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_worldBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_resolutionBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_reflectionMatrixBuffer;

    DirectX::XMMATRIX                          m_reflectView;
    WaterBuffer                                m_waterData;
    ConstantBuffer::WorldBuffer                m_worldData;
    ConstantBuffer::ResolutionBuffer           m_resolutionData;
    ConstantBuffer::ReflectionMatrixBuffer     m_reflectionMatrix;

    ID3D11ShaderResourceView*                  m_waterNormalSRV;
    ID3D11ShaderResourceView*                  m_waterWaveNormalSRV;
    ID3D11ShaderResourceView*                  m_flowSRV;
    ID3D11ShaderResourceView*                  temp;
    ID3D11SamplerState*                        m_linearSampler;
    Transform                                  m_transform;
}; // Water