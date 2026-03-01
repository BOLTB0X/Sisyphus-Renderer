#pragma once
#include "Shader.h"
#include <DirectXMath.h>

class StoneShader : public Shader {
public:
    struct RenderParams {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMFLOAT3 camPos;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT3 lightDir;
    };

    StoneShader();
    virtual ~StoneShader();

    virtual bool Init(ID3D11Device*, HWND, const std::wstring&, const std::wstring&) override;
    bool         Render(ID3D11DeviceContext*, const RenderParams&);
    void         SetSampler(ID3D11SamplerState*);

private:
    bool InitBuffer(ID3D11Device*);
    bool UpdateCameraBuffer(ID3D11DeviceContext*, DirectX::XMMATRIX, DirectX::XMMATRIX, DirectX::XMMATRIX, DirectX::XMFLOAT3);
    bool UpdateLightBuffer(ID3D11DeviceContext*, DirectX::XMFLOAT4, DirectX::XMFLOAT3);
private:
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cameraBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_lightBuffer;
    ID3D11SamplerState*                        m_sampleState;
}; // StoneShader