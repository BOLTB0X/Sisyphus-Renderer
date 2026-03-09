#include "Pch.h"
#include "ShaderHelper.h"
#include "Helpers/DebugHelper.h"
#include <d3dcompiler.h>
#include <wrl/client.h>

namespace ShaderHelper {

    bool CompileShader(HWND hwnd, const std::wstring& path, const std::string& entryPoint,
        const std::string& profile, ID3DBlob** outBlob) {
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        HRESULT hr = D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint.c_str(), profile.c_str(), flags, 0, outBlob, errorBlob.GetAddressOf());

        if (FAILED(hr)) {
            if (errorBlob) {
                OutputError(errorBlob.Get(), hwnd, path);
                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            }
            return false;
        }
        return true;
    } // CompileShader

    void OutputError(ID3DBlob* errorMsg, HWND hwnd, const std::wstring& path) {
        char* compileErrors = (char*)(errorMsg->GetBufferPointer());
        unsigned long long bufferSize = errorMsg->GetBufferSize();

        std::ofstream fout("shader-error.txt");
        for (unsigned long long i = 0; i < bufferSize; i++)
            fout << compileErrors[i];
        fout.close();

        errorMsg->Release();

        MessageBoxW(hwnd, L"compiling shader 에러. shader-error.txt 확인 요망", path.c_str(), MB_OK);
    } // OutputError

    bool InitVertexShader(ID3D11Device* device, HWND hwnd, const std::wstring& path,
        D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements,
        ID3D11VertexShader** outShader, ID3D11InputLayout** outLayout) {

        Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;

        if (!CompileShader(hwnd, path, entryPoint, vsProfile, vsBlob.GetAddressOf())) {
            return false;
        }

        if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, outShader))) {
            return false;
        }

        if (layoutDesc != nullptr && numElements > 0 && outLayout != nullptr) {
            if (FAILED(device->CreateInputLayout(layoutDesc, 
                numElements,
                vsBlob->GetBufferPointer(), 
                vsBlob->GetBufferSize(),
                outLayout))) {
                return false;
            }
        }
     
        return true;
    } // InitVertexShader

    bool InitPixelShader(ID3D11Device* device, HWND hwnd, const std::wstring& path,
        ID3D11PixelShader** outShader) {
        Microsoft::WRL::ComPtr<ID3DBlob> psBlob;

        if (!CompileShader(hwnd, path, entryPoint, psProfile, psBlob.GetAddressOf())) {
            return false;
        }

        if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, outShader))) {
            return false;
        }
        return true;
    } // InitPixelShader


    bool InitComputingShader(ID3D11Device* device, HWND hwnd, const std::wstring& path,
        ID3D11ComputeShader** outShader) {
        Microsoft::WRL::ComPtr<ID3DBlob> csBlob;

        if (!CompileShader(hwnd, path, entryPoint, csProfile, csBlob.GetAddressOf())) {
            return false;
        }

        if (FAILED(device->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, outShader))) {
            return false;
        }

        return true;
    } // InitComputingShader

} // namespace ShaderHelper