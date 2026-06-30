#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <directxmath.h>
#include "Utils/SharedConstants/BuffersConstants.h"

class VolumeTexture;
class Texture;

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

    struct Generate2DParams {
        ID3D11Device* device;
        UINT          width;
        UINT          height;
        Texture*      outputTexture;
        Generate2DParams() : device(nullptr), width(0), height(0), outputTexture(nullptr) {
        }
    }; // Generate2DParams

    struct Generate3DParams {
        VolumeTexture*     target;
        DirectX::XMFLOAT3  resolution;

        Generate3DParams() : target(nullptr), resolution(0.0f, 0.0f, 0.0f) {
        }
	}; // Generate3DParams

public:
    NoiseGenerator();
    ~NoiseGenerator();

    bool Init(const InitParams&);
    void Generate2D(ID3D11DeviceContext*, const Generate2DParams&);
    void GenerateVolume(ID3D11DeviceContext*, const Generate3DParams&);

    ID3D11ShaderResourceView* GetSRV();
private:
    Microsoft::WRL::ComPtr<ID3D11ComputeShader>       m_computeShader;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>           m_gpuTex;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>           m_stagingTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  m_srv;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_uav;
    int                                               m_grupeSize;
}; // NoiseGenerator