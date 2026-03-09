#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>

class Triangle {
public:
    struct ColorVertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    }; // ColorVertex

public:
    Triangle();
    ~Triangle();

    bool Init(ID3D11Device*, HWND);
    void Shutdown();
    void Render(ID3D11DeviceContext*);

private:
    bool InitBuffers(ID3D11Device*);
    void RenderBuffers(ID3D11DeviceContext*);
	void RenderShader(ID3D11DeviceContext*, int, DirectX::XMMATRIX, DirectX::XMMATRIX, DirectX::XMMATRIX);

private:
    // mesh
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    // shader
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_matrixBuffer;

    int m_vertexCount;
    int m_indexCount;
}; // Triangle