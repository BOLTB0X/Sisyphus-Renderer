#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <wrl/client.h>
#include "Transform.h"
#include "Resources/ConstantBufferType.h"

class DefaultMesh;

class Atmosphere {
public:
    struct RenderParams {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMFLOAT3 camPos;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT3 lightDir;
    };

public:
    Atmosphere();
    ~Atmosphere();

    bool Init(ID3D11Device*, ID3D11DeviceContext*, HWND);
    void Render(ID3D11DeviceContext*, const RenderParams&);

    void SetPosition(const DirectX::XMFLOAT3&);
    void SetPosition(float, float, float);
    void SetScale(const DirectX::XMFLOAT3&);
    void SetScale(float, float, float);

private:
	bool InitShader(ID3D11Device*, HWND);
	bool UpdateMatrixBuffer(ID3D11DeviceContext*, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&);
	bool UpdateLightBuffer(ID3D11DeviceContext*, const DirectX::XMFLOAT4&, const DirectX::XMFLOAT3&);
	bool UpdateAtmosphereBuffer(ID3D11DeviceContext*, const DirectX::XMFLOAT4&, const DirectX::XMFLOAT4&);
    bool RenderShader(ID3D11DeviceContext*, const RenderParams&);

private:
	// Resources
    std::unique_ptr<DefaultMesh>          m_mesh;
    DirectX::XMFLOAT4                 m_zenithColor;
    DirectX::XMFLOAT4                 m_horizonColor;
    Transform                         m_transform;

	// Shader resources
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_matrixBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_lightBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_atmosphereBuffer;
    ConstantBuffer::MatrixBuffer               m_prevMatrixData;
    ConstantBuffer::LightBuffer                m_prevLightData;
    ConstantBuffer::AtmosphereBuffer           m_prevAtmosphereData;
}; // Atmosphere