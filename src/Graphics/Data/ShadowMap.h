#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <directxmath.h>
#include "Resources/ConstantBufferType.h"

class RenderTexture;

class ShadowMap {
public:
    struct InitParams {
        ID3D11Device* device;
        HWND          hwnd;

        InitParams() : device(nullptr), hwnd(nullptr) {
		}
    }; // InitParams

    struct RenderParams {
        DirectX::XMMATRIX worldMatrix;
        DirectX::XMMATRIX viewMatrix;
        DirectX::XMMATRIX projectionMatrix;
    }; // RenderParams

public:
    ShadowMap();
    ~ShadowMap();

    bool Init(const InitParams&);
    bool Render(ID3D11DeviceContext*, const RenderParams&);
    void ClearShadowDepth(ID3D11DeviceContext*);

    RenderTexture*            GetShadowRT() const;
    const D3D11_VIEWPORT&     GetViewport() const;
    ID3D11DepthStencilView*   GetDSV();
    ID3D11ShaderResourceView* GetSRV();

private:
    bool InitShader(ID3D11Device*, HWND);
    bool UpdateMatrixBuffer(ID3D11DeviceContext*, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&);

private:
    std::unique_ptr<RenderTexture>             m_shadowRT;
    D3D11_VIEWPORT                             m_shadowViewport;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_matrixBuffer;
    ConstantBuffer::MatrixBuffer               m_prevMatrixBufferData;
}; // ShadowMap