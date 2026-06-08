#pragma once
#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <wrl/client.h>
#include "Components/ActorObject.h"
#include "Components/Animator.h"
#include "Resources/AssimpModel.h"
#include "Data/ShadowMap.h"

class TextureManager;

class Rakshasa : public AssimpModel, public ActorObject {
public:
    struct RenderParams {
        DirectX::XMMATRIX world;

        RenderParams() : world(DirectX::XMMatrixIdentity()) {
        }

    }; // RenderParams

public:
    Rakshasa();
    virtual ~Rakshasa();

    bool Init(const InitParams&) override;
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void DrawIndexed(ID3D11DeviceContext*);
    void Update(float);
    void OnGui();

    DirectX::XMMATRIX GetWorldMatrix() override;

private:
    struct WorldBuffer {
        DirectX::XMMATRIX world;

        WorldBuffer() {
            world = DirectX::XMMatrixIdentity();
        }
    }; // WorldBuffer;

    struct BoneBuffer {
        DirectX::XMMATRIX boneMatrices[256];

        BoneBuffer() {
            for (int i = 0; i < 100; ++i) {
                boneMatrices[i] = DirectX::XMMatrixIdentity();
            }
        }
	}; // BoneBuffer

private:
    bool InitShader(ID3D11Device*, HWND, const std::wstring&, const std::wstring&) override;

private:
    std::shared_ptr<TextureManager>            m_textureMgr;
    ID3D11SamplerState*                        m_linerSampler;
    Animator                                   m_animator;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_worldBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_boneBuffer;

    WorldBuffer                                m_worldData;
}; // Rakshasa