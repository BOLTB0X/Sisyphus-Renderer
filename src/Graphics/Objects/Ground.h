#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include "Components/Transform.h"
#include "Resources/ConstantBufferType.h"
#include "Utils/SharedConstants/BuffersConstants.h"

class DefaultMesh;

class Ground {
public:
    struct InitParams {
        ID3D11Device* device;
        HWND          hwnd;

		InitParams() : device(nullptr), hwnd(nullptr) {
        }
    }; // InitParams

    struct RenderParams {
        DirectX::XMFLOAT3         cameraPosition;
        float                     time;
		ID3D11ShaderResourceView* shadowSRV;
		ID3D11SamplerState*       shadowSampler;

        RenderParams() : cameraPosition(0.0f, 0.0f, 0.0f), time(0.0f),
            shadowSRV(nullptr), shadowSampler(nullptr) {
        }
    }; // RenderParams

public:
    Ground();
    ~Ground();

    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void DrawIndexed(ID3D11DeviceContext*);

    void              OnGui();
    DirectX::XMMATRIX GetWorldMatrix();

private:
    struct GroundBuffer {
        DirectX::XMFLOAT3 darkSand;
        float             padding1;
        DirectX::XMFLOAT3 lightSand;
        float             padding2;

        GroundBuffer() {
            darkSand = SharedConstants::BuffersConstants::DARK_SAND;
            padding1 = 0.0f;
            lightSand = SharedConstants::BuffersConstants::LIGHT_SAND;
            padding2 = 0.0f;
        }
    }; // GroundBuffer

private:
    struct WorldBuffer {
        DirectX::XMMATRIX world;

        WorldBuffer() {
            world = DirectX::XMMatrixIdentity();
        }
    }; // WorldBuffer;

private:
    bool InitShader(ID3D11Device*, HWND);

    bool UpdateGroundBuffer(ID3D11DeviceContext*);
    bool UpdateShadowBuffer(ID3D11DeviceContext*, const DirectX::XMMATRIX&);

private:
    std::unique_ptr<DefaultMesh>               m_mesh;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_worldBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_groundBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_shadowBuffer;

    WorldBuffer                                m_worldData;
    GroundBuffer                               m_GoundData;
    GroundBuffer                               m_prevGoundData;
    ConstantBuffer::ShadowBuffer               m_ShadowData;
    ConstantBuffer::ShadowBuffer               m_prevShadowData;
    Transform                                  m_transform;
}; // Ground