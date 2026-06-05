#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <wrl/client.h>
#include "Components/ActorObject.h"
#include "Data/ShadowMap.h"
#include "Resources/AssimpModel.h"
//#include "Resources/ConstantBufferType.h"

//class TextureManager;
class D3D11State;

class Tree : public AssimpModel, public ActorObject {
public:
    //struct InitParams {
    //    ID3D11Device*                   device;
    //    ID3D11DeviceContext*            context;
    //    HWND                            hwnd;
    //    std::shared_ptr<TextureManager> textMgr;
    //    std::string                     path;
    //    ID3D11SamplerState*             linerSampler;

    //    InitParams() : device(nullptr), context(nullptr), hwnd(nullptr),
    //        textMgr(nullptr), path(""), linerSampler(nullptr) {
    //    }
    //}; // InitParams

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
    Tree();
    virtual ~Tree();

    bool Init(const InitParams&) override;
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void RenderShadow(ID3D11DeviceContext*, const RenderShadowParams&);
    void OnGui();

    DirectX::XMMATRIX GetWorldMatrix() override;
    
private:
    struct WorldBuffer {
        DirectX::XMMATRIX world;

        WorldBuffer() {
            world = DirectX::XMMatrixIdentity();
        }
    }; // WorldBuffer;

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

    WorldBuffer                                m_worldData;
	CheckTransparentBuffer                     m_checkTranspData;

    std::vector<std::string>                   m_leafKeywords;
}; // Tree