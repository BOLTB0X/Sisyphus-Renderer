#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <vector>

enum class ShaderType {
    None, Color, Stone
};

class Shader {
public:
    Shader();
    Shader(const Shader&) = delete;
    virtual ~Shader() = default;

    virtual bool Init(ID3D11Device*, HWND, const std::wstring&, const std::wstring&) = 0;

    protected:
    bool Compile(ID3D11Device*, HWND, const std::wstring&, LPCSTR, LPCSTR, ID3DBlob**);
    void OutputError(ID3DBlob*, HWND, const std::wstring&);
    ShaderType GetShaderType() const;

protected:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_layout;
    ShaderType m_type;
}; // Shader