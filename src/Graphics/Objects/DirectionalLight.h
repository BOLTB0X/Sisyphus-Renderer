#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

class DirectionalLight {
public:
    DirectionalLight();
    ~DirectionalLight();

    bool Init(ID3D11Device*, HWND);
    void Update();
    void Rotate(float);
    void OnGui();

    // Setter
    void              SetLookAt(DirectX::XMFLOAT3);
    void              SetLookAt(float, float, float);
    // Getter
    DirectX::XMFLOAT3 GetPosition() const;
    DirectX::XMFLOAT3 GetDirection() const;
    DirectX::XMFLOAT4 GetDiffuse() const;
    DirectX::XMFLOAT4 GetAmbient() const;
    DirectX::XMFLOAT3 GetLookAt() const;
    DirectX::XMMATRIX GetViewMatrix() const;
    DirectX::XMMATRIX GetProjection() const;

private:
    DirectX::XMFLOAT3 m_direction;
    DirectX::XMFLOAT4 m_ambient;
    DirectX::XMFLOAT4 m_diffuse;
    DirectX::XMFLOAT3 m_lookAt;
    DirectX::XMMATRIX m_viewMatrix;
    DirectX::XMMATRIX m_projectionMatrix;
    float             m_rotationSpeed;
}; // DirectionalLight