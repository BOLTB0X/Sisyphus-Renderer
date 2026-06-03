#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

class CubeMap {
public:
    struct CubeVertex {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT2 texCoord;
    }; // CubeVertex

public:
    CubeMap();
    ~CubeMap();

    void Init(ID3D11Device*, UINT, UINT);
    void RenderBuffer(ID3D11DeviceContext*);

    ID3D11ShaderResourceView* GetSRV() const;
    ID3D11RenderTargetView* GetRTV(int) const;
    const DirectX::XMMATRIX& GetViewMatrix(int) const;
    const DirectX::XMMATRIX& GetProjMatrix() const;
    UINT GetIndexCount() const;

private:
    void CreateRenderTarget(ID3D11Device*);
    void CreateGeometry(ID3D11Device*);
    void InitMatrices();

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_colorTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_colorRTV[6];
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_colorSRV;

    // 베이킹용 내부 매쉬
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    // 큐브맵 면을 위한 행렬
    DirectX::XMMATRIX                    m_viewMatrices[6];
    DirectX::XMMATRIX                    m_projMatrix;

    UINT                                 m_width;
    UINT                                 m_height;
    UINT                                 m_indexCount;
}; // CubeMap