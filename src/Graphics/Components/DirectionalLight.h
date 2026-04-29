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
    DirectX::XMFLOAT4 GetSunset() const;
    DirectX::XMFLOAT4 GetLight() const;
    DirectX::XMFLOAT3 GetLookAt() const;
    DirectX::XMMATRIX GetViewMatrix() const;
    DirectX::XMMATRIX GetProjection() const;
    DirectX::XMFLOAT2 GetUV(const DirectX::XMMATRIX&, const DirectX::XMMATRIX&) const;

private:
    DirectX::XMFLOAT3 m_direction;
    DirectX::XMFLOAT4 m_ambient;
    DirectX::XMFLOAT4 m_diffuse;
    DirectX::XMFLOAT4 m_sunset;
    DirectX::XMFLOAT4 m_night;
    DirectX::XMFLOAT3 m_lookAt;
    DirectX::XMMATRIX m_viewMatrix;
    DirectX::XMMATRIX m_projectionMatrix;
    float             m_rotationSpeed;
}; // DirectionalLight