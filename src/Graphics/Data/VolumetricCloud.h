#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include <string>

class RenderTexture;

class VolumetricCloud {
public:
    struct InitParams {
        ID3D11Device*             device;
        HWND                      hwnd;
        ID3D11ShaderResourceView* weatherMapSRV;
        ID3D11ShaderResourceView* baseNoiseSRV;
        ID3D11ShaderResourceView* detailNoiseSRV;
        ID3D11ShaderResourceView* blueNoiseSRV;
        ID3D11SamplerState*       sampler;

		InitParams() : device(nullptr), hwnd(0), weatherMapSRV(nullptr),
            baseNoiseSRV(nullptr), detailNoiseSRV(nullptr), blueNoiseSRV(nullptr),
            sampler(nullptr) {
        }
    }; // InitParams

    struct RenderParams {
        float                     time;
        ID3D11ShaderResourceView* LutSRV;

        RenderParams() : 
            time(0.0f), LutSRV(nullptr) {
        }
    }; // RenderParams

public:
    VolumetricCloud();
    ~VolumetricCloud();

    bool Init(const InitParams&);
    void Execute(ID3D11DeviceContext*, const RenderParams&);
    void OnGui();

    ID3D11ShaderResourceView* GetCloudSRV();

private:
    struct CloudBuffer {
        // Row 1
        float             cloudMinHeight;
        float             cloudMaxHeight;
    };

    struct ResolutionBuffer {
        DirectX::XMFLOAT2 resolution;
        DirectX::XMFLOAT2 padding;

        ResolutionBuffer() : resolution(1024.0f, 512.0f), padding(0.0f, 0.0f) {
        }
    }; // ResolutionBuffer

private:
    std::unique_ptr<RenderTexture>              m_resultRT;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_cloudBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_resolutionBuffer;
    // buffer
    CloudBuffer                                 m_cloudBufferData;
    ResolutionBuffer                            m_resolutionData;
    CloudBuffer                                 m_prevCloudBufferData;
    ResolutionBuffer                            m_prevResolutionData;
    // srv
    ID3D11SamplerState*                         m_linerWrapSampler;
    ID3D11ShaderResourceView*                   m_WeatherMapSRV;
    ID3D11ShaderResourceView*                   m_baseNoiseSRV;
    ID3D11ShaderResourceView*                   m_detailNoiseSRV;
    ID3D11ShaderResourceView*                   m_blueNoiseSRV;
}; // VolumetricCloud