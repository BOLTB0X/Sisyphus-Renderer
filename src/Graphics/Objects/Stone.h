#pragma once
#include "Resources/AssimpModel.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

class StoneShader;

class Stone : public AssimpModel {
public:
    struct RenderParams {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMFLOAT3 camPos;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT3 lightDir;
    };

    Stone();
    virtual ~Stone();
    
    bool              Init(ID3D11Device*, ID3D11DeviceContext*, HWND, const std::string&);
    void              Render(ID3D11DeviceContext*, const RenderParams&);
    DirectX::XMMATRIX GetWorldMatrix() const;
    void              SetPosition(float, float, float);
    void              SetSampler(ID3D11SamplerState*);

private:
    std::unique_ptr<StoneShader> m_shader;
    DirectX::XMMATRIX            m_worldMatrix;
    ID3D11SamplerState*          m_sampler;
}; // Stone