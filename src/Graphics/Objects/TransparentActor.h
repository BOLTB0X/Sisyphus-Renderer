#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <wrl/client.h>
#include "Data/ShadowMap.h"
#include "Resources/ActorObject.h"
#include "Resources/AssimpModel.h"
#include "Resources/ConstantBuffer.h"

class TextureManager;
class D3D11State;
class RenderQueue;

class TransparentActor : public AssimpModel, public ActorObject {
public:
    struct RenderParams {
        DirectX::XMMATRIX world;
        D3D11State*       states;

        RenderParams() : world(DirectX::XMMatrixIdentity()), states(nullptr) {
        }

    }; // RenderParams

    struct RenderShadowParams {
        ShadowMap* shadowMap = nullptr;
        ShadowMap::RenderParams* shadowParams = nullptr;
        D3D11State* states = nullptr;

        RenderShadowParams() = default;
    }; // RenderShadowParams
   
public:
    TransparentActor();
    virtual ~TransparentActor();

    bool Init(const InitParams&) override;
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void RenderShadow(ID3D11DeviceContext*, const RenderShadowParams&);
    void Submit(const SubmitParams& params) override;
    void OnGui();

    DirectX::XMMATRIX GetWorldMatrix() override;
    
private:
    struct CheckTransparentBuffer {
        int               isLeaf;
        DirectX::XMFLOAT3 padding;

        CheckTransparentBuffer() {
            isLeaf = 0;
            padding = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        }
    }; // CheckTransparentBuffer

private:
    bool InitShader(ID3D11Device*, HWND, const std::wstring&, const std::wstring&) override;
    bool IsTransparentMaterial(const std::string&) const;

private:
    std::shared_ptr<TextureManager>            m_textureMgr;
    ID3D11SamplerState*                        m_linerSampler;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_worldBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_checkLeafBuffer;

    ConstantBuffer::WorldBuffer                m_worldData;
	CheckTransparentBuffer                     m_checkTranspData;

    std::vector<std::string>                   m_leafKeywords;
}; // TransparentActor