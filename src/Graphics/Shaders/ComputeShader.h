#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>

class ComputeShader {
public:
    ComputeShader();
    virtual ~ComputeShader() = default;

    virtual bool Init(ID3D11Device*, HWND, const std::wstring&) = 0;

protected:
    bool Compile(ID3D11Device*, HWND, const std::wstring&, LPCSTR, LPCSTR, ID3DBlob**);
    void OutputError(ID3DBlob*, HWND, const std::wstring&);

    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
}; // ComputeShader