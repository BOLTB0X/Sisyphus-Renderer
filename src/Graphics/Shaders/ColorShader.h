#pragma once
#include "Shader.h"
#include <directxmath.h>

class ColorShader : public Shader {
public:
    ColorShader();
    ColorShader(const ColorShader& other) = delete;
    virtual ~ColorShader() = default;
    virtual bool Init(ID3D11Device*, HWND, const std::wstring&, const std::wstring&) override;

    bool Render(ID3D11DeviceContext*, int, DirectX::XMMATRIX, DirectX::XMMATRIX, DirectX::XMMATRIX);

private:
    bool InitBuffer(ID3D11Device*);
    bool UpdateMatrixBuffer(ID3D11DeviceContext*, DirectX::XMMATRIX, DirectX::XMMATRIX, DirectX::XMMATRIX);

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;
}; // Shader