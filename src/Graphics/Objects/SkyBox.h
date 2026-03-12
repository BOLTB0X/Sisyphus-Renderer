#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include "Atmosphere.h"
#include "Resources/ConstantBufferType.h"

class DefaultMesh;
class D3D11State;

class SkyBox {
public:
    struct RenderParams {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMFLOAT3 lightDir;
    }; // RenderParams

public:
    SkyBox();
    ~SkyBox();

    bool Init(ID3D11Device*, ID3D11DeviceContext*, HWND, ID3D11SamplerState*);
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void UpdateAtmosphere(ID3D11DeviceContext*, D3D11State*, const Atmosphere::RenderParams&);

private:
    bool InitShader(ID3D11Device*, HWND);
    bool UpdateMatrixBuffer(ID3D11DeviceContext*, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&);

private:
    std::unique_ptr<Atmosphere>  m_atmosphere;
    std::unique_ptr<DefaultMesh> m_cubeMesh;
    // resources
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_matrixBuffer;
    ID3D11SamplerState*                        m_sampler;
    ConstantBuffer::MatrixBuffer               m_prevMatrixData;
}; // SkyBox