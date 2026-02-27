#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>

class ColorShader;

class Triangle {
public:
    Triangle();
    ~Triangle();

    bool Init(ID3D11Device*, HWND);
    void Shutdown();
    void Render(ID3D11DeviceContext*);

private:
    bool InitBuffers(ID3D11Device*);
    void RenderBuffers(ID3D11DeviceContext*);

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    std::unique_ptr<ColorShader>         m_colorShader;

    int m_vertexCount;
    int m_indexCount;
}; // Triangle