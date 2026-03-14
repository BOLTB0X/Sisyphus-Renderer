#pragma once
#include <d3d11.h>
#include <string>

namespace ShaderHelper {
	inline static std::string entryPoint = "main";
	inline static std::string vsProfile = "vs_5_0";
	inline static std::string psProfile = "ps_5_0";
    inline static std::string csProfile = "cs_5_0";

    bool CompileShader(HWND, const std::wstring&, const std::string&, const std::string&, ID3DBlob**);
    void OutputError(ID3DBlob*, HWND, const std::wstring&);
    bool InitVertexShader(ID3D11Device*, HWND, const std::wstring&, D3D11_INPUT_ELEMENT_DESC*, UINT, ID3D11VertexShader**, ID3D11InputLayout**);
    bool InitPixelShader(ID3D11Device*, HWND, const std::wstring&, ID3D11PixelShader**);
    bool InitComputingShader(ID3D11Device*, HWND, const std::wstring&, ID3D11ComputeShader**);

    template<typename T>
    bool InitConstantBuffer(ID3D11Device* device, ID3D11Buffer** outBuffer) {
        D3D11_BUFFER_DESC cbd = {};
        cbd.Usage = D3D11_USAGE_DYNAMIC;
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbd.ByteWidth = sizeof(T);

        //if (cbd.ByteWidth % 16 != 0) {
        //    cbd.ByteWidth = (cbd.ByteWidth / 16 + 1) * 16;
        //}

        return SUCCEEDED(device->CreateBuffer(&cbd, nullptr, outBuffer));
    } // InitConstantBuffer

    // 상수 버퍼 업데이트
    template<typename T>
    bool UpdateConstantBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, const T& data) {
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (FAILED(context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            return false;
        }
        memcpy(mapped.pData, &data, sizeof(T));
        context->Unmap(buffer, 0);
        return true;
    } // UpdateConstantBuffer

} // ShaderHelper