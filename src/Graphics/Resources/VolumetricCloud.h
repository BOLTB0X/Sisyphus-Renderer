#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <wrl/client.h>
#include "Resources/DefaultMesh.h"
#include "Resources/ConstantBufferType.h"

class VolumetricCloud {
public:
    struct RenderParams {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMFLOAT3 camPos;

        DirectX::XMFLOAT3 planetCenter; // 보통 (0, -6360000, 0)
        float planetRadius; // 보통 6360000 (6360km)
        float cloudMinHeight;          // 구름 시작 고도 (예: 1500m)
        float cloudMaxHeight;          // 구름 끝 고도 (예: 4000m)
        DirectX::XMFLOAT3 LightDir;
    };

    VolumetricCloud();
    ~VolumetricCloud();

    bool Init(ID3D11Device* device, HWND hwnd);
    void Render(ID3D11DeviceContext* context, const RenderParams& params);

private:
    bool InitShader(ID3D11Device* device, HWND hwnd);
	bool UpdateCloudBuffer(ID3D11DeviceContext*, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMFLOAT3&);
    bool RenderShader(ID3D11DeviceContext*, const RenderParams&);

private:
    std::unique_ptr<DefaultMesh> m_mesh;

    // shader resources
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_rayBuffer;
	ConstantBuffer::CloudBuffer                  m_preRayBuffer;
}; // VolumetricCloud