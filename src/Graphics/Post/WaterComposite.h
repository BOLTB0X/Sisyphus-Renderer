#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include <string>
// Resources
#include "Resources/ConstantBuffer.h"
// Utils
#include "SharedConstants/BuffersConstants.h"

class RenderTexture;

class WaterComposite {
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
        ID3D11ShaderResourceView* reflectionSRV;
        ID3D11ShaderResourceView* sceneSRV;
        ID3D11ShaderResourceView* sceneDepthSRV;

        RenderParams() : reflectionSRV(nullptr), sceneSRV(nullptr),
            sceneDepthSRV(nullptr) {
        }
    }; // RenderParams

public:
    WaterComposite();
    ~WaterComposite();

    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void OnGui();

    void                      ClearRT(ID3D11DeviceContext*);
    ID3D11Texture2D*          GetTexture() const;
    ID3D11RenderTargetView*   GetRTV() const;
    ID3D11ShaderResourceView* GetSRV() const;
    float                     GetWaterHeight();

private:
    struct WaterBuffer {
        float             waterHeight;
        DirectX::XMFLOAT3 padding1;

        DirectX::XMFLOAT3 waterColorShallow;
        float             padding2;

        DirectX::XMFLOAT3 waterColorDeep;
        float             padding3;

        float             distortion;
        float             reflectivity;
        float             density;
        float             padding4;

        WaterBuffer() {
            waterHeight = 0.0f;
            padding1 = { 0.0f, 0.0f, 0.0f };
            waterColorShallow = SharedConstants::BuffersConstants::WATER_COLOR_SHALLOW;
            padding2 = 0.0f;
            waterColorDeep = SharedConstants::BuffersConstants::WATER_COLOR_DEEP;
            padding3 = 0.0f;
            distortion = SharedConstants::BuffersConstants::WATER_DISTORTION;
            reflectivity = SharedConstants::BuffersConstants::WATER_REFLECTIVITY;
            density = SharedConstants::BuffersConstants::WATER_DENSITY;
            padding4 = 0.0f;
        }
    }; // WaterBuffer

private:
    bool InitShader(ID3D11Device*, HWND, const int&, const int&);

private:
    std::unique_ptr<RenderTexture>             m_compositeRT;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_waterBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_resolutionBuffer;

    WaterBuffer                                m_waterBufferData;
    ConstantBuffer::ResolutionBuffer           m_resolutionBufferData;

    float                                      m_waterHeight;
    ID3D11ShaderResourceView*                  m_waterNormalSRV;
    ID3D11ShaderResourceView*                  m_waterWaveNormalSRV;
    ID3D11ShaderResourceView*                  m_flowSRV;
    ID3D11SamplerState*                        m_linearSampler;
    ID3D11ShaderResourceView*                  temp;
}; // WaterComposite