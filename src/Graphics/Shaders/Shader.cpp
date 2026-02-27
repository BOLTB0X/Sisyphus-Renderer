#include "Pch.h"
#include "Shader.h"
// STL
#include <d3dcompiler.h>
#include <fstream>


Shader::Shader()
{
    m_type = ShaderType::None;
} // Shader


bool Shader::Compile(ID3D11Device* device,
    HWND hwnd,
    const std::wstring& path,
    LPCSTR entry,
    LPCSTR profile,
    ID3DBlob** blob)
{
    ID3DBlob* errorBlob = nullptr;

    HRESULT result = D3DCompileFromFile(
        path.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entry,
        profile,
        D3D10_SHADER_ENABLE_STRICTNESS,
        0,
        blob,
        &errorBlob
    );

    if (FAILED(result))
    {
        if (errorBlob)
            OutputError(errorBlob, hwnd, path);
        else 
            MessageBoxW(hwnd, path.c_str(), L"셰이더 파일이 없음", MB_OK);
        return false;
    }

    return true;
} // Compile


void Shader::OutputError(
    ID3DBlob* errorMsg,
    HWND hwnd,
    const std::wstring& path)
{
    char* compileErrors = (char*)(errorMsg->GetBufferPointer());
    unsigned long long bufferSize = errorMsg->GetBufferSize();

    std::ofstream fout("shader-error.txt");
    for (unsigned long long i = 0; i < bufferSize; i++)
        fout << compileErrors[i];
    fout.close();

    errorMsg->Release();

    MessageBoxW(hwnd, L"compiling shader 에러. shader-error.txt 확인 요망", path.c_str(), MB_OK);
} // OutputError


ShaderType Shader::GetShaderType() const
{
    return m_type;
} // GetShaderType

