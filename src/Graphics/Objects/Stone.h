#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include "Transform.h"
#include "Resources/AssimpModel.h"

class StoneShader;
class TextureManager;

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
    
    bool              Init(ID3D11Device*, ID3D11DeviceContext*, HWND, std::shared_ptr<TextureManager>, const std::string&);
    void              Render(ID3D11DeviceContext*, const RenderParams&);

	void 			  SetPosition(const DirectX::XMFLOAT3&);
    void              SetPosition(float, float, float);
	void 			  SetRotation(const DirectX::XMFLOAT3&);
	void              SetRotation(float, float, float);
	void 			  SetScale(const DirectX::XMFLOAT3&);
	void              SetScale(float, float, float);
    void              SetSampler(ID3D11SamplerState*);

	void              Translate(const DirectX::XMFLOAT3&);
	void              Translate(float, float, float);
	void              Rotate(const DirectX::XMFLOAT3&);
	void              Rotate(float, float, float);

    DirectX::XMMATRIX GetWorldMatrix();
    unsigned int 	  GetRenderCount() const;
private:
    std::unique_ptr<StoneShader>    m_shader;
    std::shared_ptr<TextureManager> m_textureMgr;
    ID3D11SamplerState*             m_sampler;
    Transform                       m_transform;
	unsigned int                    m_RenderCount;
}; // Stone