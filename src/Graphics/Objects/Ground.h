#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include "Resources/ConstantBufferType.h"
#include "Resources/Transform.h"
#include "Utils/SharedConstants/BuffersConstants.h"

class DefaultMesh;

class Ground {
public:
    struct InitParams {
        ID3D11Device* device;
        HWND          hwnd;
    }; // InitParams

    struct RenderParams {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMFLOAT3 cameraPosition;
        DirectX::XMFLOAT3 lightDir;
        DirectX::XMFLOAT4 lightDiffuse;
        float             time;
        DirectX::XMMATRIX lightView;
        DirectX::XMMATRIX lightProjection;
    }; // RenderParams

public:
    Ground();
    ~Ground();

    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void DrawIndexed(ID3D11DeviceContext*);

    void              OnGui();
    DirectX::XMMATRIX GetWorldMatrix();
    void              SetShadowMap(ID3D11ShaderResourceView*);
    void              SetShadowSampler(ID3D11SamplerState*);

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
    bool InitShader(ID3D11Device*, HWND);
    bool UpdateCommonBuffer(ID3D11DeviceContext*,
        const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&,
        const DirectX::XMFLOAT3&, const DirectX::XMFLOAT3&, const DirectX::XMFLOAT4&);
    bool UpdateGroundBuffer(ID3D11DeviceContext*);
    bool UpdateShadowBuffer(ID3D11DeviceContext*, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&);

private:
    std::unique_ptr<DefaultMesh>               m_mesh;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_commonBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_groundBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_shadowBuffer;

    GroundBuffer                               m_GoundData;
    GroundBuffer                               m_prevGoundData;
    ConstantBuffer::CommonBuffer               m_CommonData;
    ConstantBuffer::CommonBuffer               m_prevCommonData;
    ConstantBuffer::ShadowBuffer               m_ShadowData;
    ConstantBuffer::ShadowBuffer               m_prevShadowData;
    Transform                                  m_transform;

    ID3D11ShaderResourceView*                  m_shadowSRV;
    ID3D11SamplerState*                        m_shadowSampler;
}; // Ground