#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include "Resources/ConstantBuffer.h"
#include "Utils/SharedConstants/BuffersConstants.h"

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
        ID3D11SamplerState*            sampler;
        ID3D11ShaderResourceView*      depth;

        InitParams() : device(nullptr), context(nullptr), hwnd(nullptr),
            sampler(nullptr), depth(nullptr) {
		}
    }; // InitParams

    struct RenderParams {
        DirectX::XMFLOAT3 camPos;
        float             time;
        ID3D11ShaderResourceView* skyLUT;
		RenderParams() : camPos(0.0f, 0.0f, 0.0f), time(0.0f), skyLUT(nullptr) {
        }
    }; // RenderParams

public:
    SkyBox();
    ~SkyBox();

    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void OnGui(); // Imgui 용

private:
    bool InitShader(ID3D11Device*, HWND);

private:
    std::unique_ptr<DefaultMesh>               m_CubeMesh;
    // shader resources
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_worldBuffer;
    // buffers
    ConstantBuffer::WorldBuffer                m_WorldData;
    // textures
    ID3D11SamplerState*                        m_linerWrapSampler;
    ID3D11ShaderResourceView*                  m_depthSRV;
}; // SkyBox