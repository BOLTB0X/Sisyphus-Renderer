#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <wrl/client.h>
#include "Data/ShadowMap.h"
#include "Components/Animator.h"
#include "Resources/ActorObject.h"
#include "Resources/AssimpModel.h"
#include "Resources/ConstantBuffer.h"

class RigidActor : public AssimpModel, public ActorObject {
public:
    struct RenderParams {
        DirectX::XMMATRIX world;
        RenderParams() : world(DirectX::XMMatrixIdentity()) {}
    }; // RenderParams

    struct RenderShadowParams {
        class ShadowMap* shadowMap = nullptr;
        struct ShadowMap::RenderParams* shadowParams = nullptr;
        class D3D11State* states = nullptr;
        RenderShadowParams() = default;
    }; // RenderShadowParams

public:
    RigidActor();
    virtual ~RigidActor();

    bool Init(const InitParams&) override;
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void RenderShadow(ID3D11DeviceContext*, const RenderShadowParams&);
    void Animate(float);
    void OnGui();

    DirectX::XMMATRIX GetWorldMatrix() override;

private:
    bool InitShader(ID3D11Device*, HWND, const std::wstring&, const std::wstring&) override;

    void RenderNode(ID3D11DeviceContext*, const AssimpModel::ModelNode*, const DirectX::XMMATRIX&, const std::unordered_map<std::string, DirectX::XMMATRIX>&);
    void RenderShadowNode(ID3D11DeviceContext*, const AssimpModel::ModelNode*, const DirectX::XMMATRIX&, const std::unordered_map<std::string, DirectX::XMMATRIX>&, const RenderShadowParams&);

private:
    std::shared_ptr<TextureManager>            m_textureMgr;
    ID3D11SamplerState*                        m_linerSampler;
    Animator                                   m_Animator;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_staticVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_worldBuffer;

    ConstantBuffer::WorldBuffer                m_worldData;
}; // RigidActor