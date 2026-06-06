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
        DirectX::XMMATRIX         worldMatrix;
        DirectX::XMMATRIX         viewMatrix;
        DirectX::XMMATRIX         projectionMatrix;

        ID3D11ShaderResourceView* alphaSRV;
        ID3D11SamplerState*       linearSampler;

        RenderParams() : worldMatrix(DirectX::XMMatrixIdentity()),
            viewMatrix(DirectX::XMMatrixIdentity()),
            projectionMatrix(DirectX::XMMatrixIdentity()),
            alphaSRV(nullptr),
            linearSampler(nullptr) {
        }
    }; // RenderParams

public:
    ShadowMap();
    ~ShadowMap();

    bool Init(const InitParams&);
    bool RenderOpaque(ID3D11DeviceContext*, const RenderParams&);
    bool RenderTransparent(ID3D11DeviceContext*, const RenderParams&);
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

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_depthVertexShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_depthLayout;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_transparentDepthVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_transparentDepthPixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_transparentLayout;

    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_matrixBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_checkLeafBuffer;

    ConstantBuffer::MatrixBuffer               m_prevMatrixBufferData;
}; // ShadowMap