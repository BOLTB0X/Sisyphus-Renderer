#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <wrl/client.h>
#include "Resources/ActorObject.h"
#include "Resources/AssimpModel.h"
#include "Resources/ConstantBuffer.h"

class RenderQueue;

class MayaActor : public AssimpModel, public ActorObject {
public:
    struct RenderParams {
        DirectX::XMMATRIX world;

        RenderParams() : world(DirectX::XMMatrixIdentity()) {
        }
    }; // RenderParams

public:
    MayaActor();
    virtual ~MayaActor();

    bool Init(const InitParams&) override;
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void DrawIndexed(ID3D11DeviceContext*);
    void Submit(const SubmitParams& params) override;
    void OnGui();

    DirectX::XMMATRIX GetScalingWorldMatrix();

private:
    bool InitShader(ID3D11Device*, HWND, const std::wstring&, const std::wstring&) override;

private:
    std::shared_ptr<TextureManager>            m_textureMgr;
    ID3D11SamplerState*                        m_wrapSampler;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_worldBuffer;

    ConstantBuffer::WorldBuffer                m_worldData;
}; // MayaActor