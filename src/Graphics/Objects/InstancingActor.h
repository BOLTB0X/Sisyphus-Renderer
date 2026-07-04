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

class InstancingActor : public AssimpModel, public ActorObject {
public:
    struct RenderParams {
        DirectX::XMMATRIX         world;
        D3D11State*               states;
        ID3D11ShaderResourceView* instanceSRV;

        RenderParams() : world(DirectX::XMMatrixIdentity()), states(nullptr), instanceSRV(nullptr) {
        }
    }; // RenderParams

    struct RenderShadowParams {
        ShadowMap*                 shadowMap;
        ShadowMap::RenderParams*   shadowParams;
        D3D11State*                states;
        ID3D11ShaderResourceView*  instanceSRV;
        ID3D11UnorderedAccessView* instanceUAV;

        RenderShadowParams() : shadowMap(nullptr), shadowParams(nullptr), states(nullptr), instanceSRV(nullptr), instanceUAV(nullptr) {
        }
    }; // RenderShadowParams

public:
    InstancingActor();
    virtual ~InstancingActor();

    bool Init(const InitParams&) override;
    void RenderShadow(ID3D11DeviceContext*, const RenderShadowParams&);
    void Submit(const SubmitParams&) override;
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
    std::shared_ptr<TextureManager>                   m_textureMgr;
    ID3D11SamplerState*                               m_linerSampler;

    Microsoft::WRL::ComPtr<ID3D11VertexShader>        m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>         m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>         m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>              m_worldBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>              m_checkLeafBuffer;

    std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> m_argsBuffers;

    ConstantBuffer::WorldBuffer                       m_worldData;
    CheckTransparentBuffer                            m_checkTranspData;

    std::vector<std::string>                          m_leafKeywords;
    ID3D11ShaderResourceView*                         m_instanceSRV;
    ID3D11UnorderedAccessView*                        m_instanceUAV;
}; // InstancingActor