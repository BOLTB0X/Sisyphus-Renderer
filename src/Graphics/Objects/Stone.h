#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <wrl/client.h>
#include "Components/Transform.h"
#include "Resources/AssimpModel.h"
#include "Resources/ConstantBufferType.h"

class TextureManager;

class Stone : public AssimpModel {
public:
    struct InitParams {
        ID3D11Device*                   device;
        ID3D11DeviceContext*            context;
        HWND                            hwnd;
        std::shared_ptr<TextureManager> textMgr;
        std::string                     path;
        ID3D11SamplerState*             linerSampler;

        InitParams() : device(nullptr), context(nullptr), hwnd(nullptr), 
            textMgr(nullptr), path(""), linerSampler(nullptr) {
		}
    }; // InitParams

    struct RenderParams {
        DirectX::XMMATRIX   world;
        RenderParams() : world(DirectX::XMMatrixIdentity()) {
		}
    }; // RenderParams

public:
    Stone();
    virtual ~Stone();
    
    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void DrawIndexed(ID3D11DeviceContext*);
    void OnGui();

	void SetPosition(const DirectX::XMFLOAT3&);
    void SetPosition(float, float, float);
	void SetRotation(const DirectX::XMFLOAT3&);
	void SetRotation(float, float, float);
	void SetScale(const DirectX::XMFLOAT3&);
	void SetScale(float, float, float);

	void Translate(const DirectX::XMFLOAT3&);
	void Translate(float, float, float);
	void Rotate(const DirectX::XMFLOAT3&);
	void Rotate(float, float, float);

    DirectX::XMMATRIX GetWorldMatrix();
    unsigned int 	  GetRenderCount() const;

private:
    struct WorldBuffer {
        DirectX::XMMATRIX world;

        WorldBuffer() {
            world = DirectX::XMMatrixIdentity();
        }
    }; // WorldBuffer;

private:
    bool InitShader(ID3D11Device*, HWND);
    bool RenderShader(ID3D11DeviceContext*, const RenderParams&);

private:
    // model resources
    std::shared_ptr<TextureManager>            m_textureMgr;
    ID3D11SamplerState*                        m_sampler;
    Transform                                  m_transform;
	unsigned int                               m_RenderCount;
    // shader resources
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_worldBuffer;
    WorldBuffer                                m_worldData;
}; // Stone