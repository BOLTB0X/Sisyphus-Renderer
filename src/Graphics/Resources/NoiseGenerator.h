#pragma once
#include <wrl/client.h>
#include <string>
#include "Resources/ConstantBufferType.h"

class VolumeTexture;

class NoiseGenerator {
public:
    NoiseGenerator();
    ~NoiseGenerator();

    bool Init(ID3D11Device*, HWND, const std::wstring& path);
    void Generate(ID3D11DeviceContext*, VolumeTexture*, const ConstantBuffer::NoiseBuffer&);

private:
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_noiseBuffer;
    ConstantBuffer::NoiseBuffer                 m_preNoiseBuffer;
}; // NoiseGenerator