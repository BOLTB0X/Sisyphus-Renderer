#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <directxmath.h>
#include <memory>

class RenderTexture;

class CloudMap {
public:
    struct InitParams {
        ID3D11Device* device;
        HWND         hwnd;

        InitParams() : device(nullptr), hwnd(nullptr) {
        }
	}; // InitParams

public:
    CloudMap();
	~CloudMap();

    bool                      Init(const InitParams&);
    void                      Generate(ID3D11DeviceContext*);
    ID3D11ShaderResourceView* GetSRV();

private:
    std::unique_ptr<RenderTexture>              m_resultRT;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
    float                                       m_gridSize;
    UINT                                        m_dispatchCount;
	UINT                                        m_resolution;
}; // CloudMap