#pragma once
#include <DirectXMath.h>

class DirectionalLight {
public:
    DirectionalLight();
    ~DirectionalLight();

    void Init();

    void OnGui();
    // Getter
    DirectX::XMFLOAT3 GetDirection() const;
    DirectX::XMFLOAT4 GetDiffuse()   const;

private:
    DirectX::XMFLOAT3 m_direction;
    DirectX::XMFLOAT4 m_diffuse;
}; // DirectionalLight