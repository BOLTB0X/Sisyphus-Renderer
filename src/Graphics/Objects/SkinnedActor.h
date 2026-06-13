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

class TextureManager;
class D3D11State;

class SkinnedActor : public AssimpModel, public ActorObject {
public:
    struct RenderParams {
        DirectX::XMMATRIX world;

        RenderParams() : world(DirectX::XMMatrixIdentity()) {
        }

    }; // RenderParams

    struct RenderShadowParams {
        ShadowMap* shadowMap = nullptr;
        ShadowMap::RenderParams* shadowParams = nullptr;
        D3D11State* states = nullptr;

        RenderShadowParams() = default;
    }; // RenderShadowParams

public:
    SkinnedActor();
    virtual ~SkinnedActor();

    bool Init(const InitParams&) override;
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void RenderShadow(ID3D11DeviceContext*, const RenderShadowParams&);
    void Animate(float);
    void OnGui();

    DirectX::XMMATRIX GetWorldMatrix() override;

private:
    struct BoneBuffer {
        DirectX::XMMATRIX boneMatrices[256];

        BoneBuffer() {
            for (int i = 0; i < 256; ++i) {
                boneMatrices[i] = DirectX::XMMatrixIdentity();
            }
        }
	}; // BoneBuffer

private:
    bool InitShader(ID3D11Device*, HWND, const std::wstring&, const std::wstring&) override;

private:
    std::shared_ptr<TextureManager>            m_textureMgr;
    ID3D11SamplerState*                        m_linerSampler;
    Animator                                   m_Animator;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_skinnedVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_worldBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_boneBuffer;

    ConstantBuffer::WorldBuffer                m_worldData;
    BoneBuffer                                 m_boneData;
}; // SkinnedActor