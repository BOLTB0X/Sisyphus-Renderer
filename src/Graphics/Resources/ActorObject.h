#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <wrl/client.h>
#include <string>
// Components
#include "Components/Transform.h"

class TextureManager;

class ActorObject {
public:
    struct InitParams {
        ID3D11Device*                   device;
        ID3D11DeviceContext*            context;
        HWND                            hwnd;
        std::shared_ptr<TextureManager> textMgr;
        std::string                     path;
		std::wstring                    VSPath;
		std::wstring                    PSPath;
        ID3D11SamplerState*             linerSampler;
        ID3D11SamplerState*             pointSampler;

        InitParams() : device(nullptr), context(nullptr), hwnd(nullptr),
            textMgr(nullptr), path(""), VSPath(L""), PSPath(L""),
            linerSampler(nullptr), pointSampler(nullptr) {
        }
    }; // InitParams

public:
    ActorObject();
    virtual ~ActorObject() = default;
	virtual bool Init(const InitParams&) = 0;
    virtual bool InitShader(ID3D11Device*, HWND, const std::wstring&, const std::wstring&) = 0;

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

    DirectX::XMFLOAT3 GetPosition() const;
    virtual DirectX::XMMATRIX GetWorldMatrix();

    unsigned int GetRenderCount() const;

protected:
    void DrawTransformGui();

    Transform    m_transform;
    unsigned int m_RenderCount;
}; // ActorObject