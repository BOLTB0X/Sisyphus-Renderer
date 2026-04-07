#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <directxmath.h>
#include "Resources/ConstantBufferType.h"

class DepthRecorder {
public:
    struct InitParams {
        ID3D11Device* device;
        HWND          hwnd;
    }; // InitParams

    struct RenderParams {
        DirectX::XMMATRIX worldMatrix;
        DirectX::XMMATRIX viewMatrix;
        DirectX::XMMATRIX projectionMatrix;
    }; // RenderParams

public:
    DepthRecorder();
    ~DepthRecorder();

    bool Init(const InitParams&);
    bool Render(ID3D11DeviceContext*, const RenderParams&);

private:
    bool InitShader(ID3D11Device*, HWND);
    bool UpdateMatrixBuffer(ID3D11DeviceContext*, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&);

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_matrixBuffer;
    ConstantBuffer::MatrixBuffer               m_prevMatrixBufferData;
}; // DepthRecorder