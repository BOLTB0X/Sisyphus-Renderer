#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <directxmath.h>
#include "Utils/SharedConstants/BuffersConstants.h"

class VolumeTexture;

class NoiseGenerator {
public:
    struct InitParams {
        ID3D11Device* device;
        HWND          hwnd;
        std::wstring  path;
        int           groupSize;

        InitParams() : device(nullptr), hwnd(nullptr), path(L""), groupSize(0) {
        }
	}; // InitParams

    struct GenerateParams {
        VolumeTexture*     target;
        DirectX::XMFLOAT3  resolution;

        GenerateParams() : target(nullptr), resolution(0.0f, 0.0f, 0.0f) {
        }
	}; // GenerateParams

public:
    NoiseGenerator();
    ~NoiseGenerator();

    bool Init(const InitParams&);
    void Generate(ID3D11DeviceContext*, const GenerateParams&);

private:
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
    int                                         m_grupeSize;
}; // NoiseGenerator