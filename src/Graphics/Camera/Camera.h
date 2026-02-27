#pragma once
#include <memory>
#include <DirectXMath.h>

class Frustum;

class Camera {
public:
    Camera();
    Camera(const Camera&) = delete;
    ~Camera();

    bool Init(float, float, float, float);
    void Update();
    void BuildFrustum();

    // 조작 함수
    void AddRotation(float, float);
    void AddPitch(float);
    void AddYaw(float);
    void AddFOV(float);

public: // Setter & Getter
    void SetPosition(const DirectX::XMFLOAT3&);
    void SetPosition(float, float, float );
    void SetRotation(const DirectX::XMFLOAT3&);
    void SetRotation(float, float, float);
    
    void SetFov(float);
    void SetAspect(float);
    void SetNear(float);
    void SetFar(float);

    DirectX::XMFLOAT3 GetPosition() const;
    DirectX::XMFLOAT3 GetRotation() const;
    DirectX::XMMATRIX GetViewMatrix() const;
    DirectX::XMMATRIX GetProjectionMatrix() const;
    Frustum* GetFrustum() const;
    
    float GetFov() const;
    float GetNear() const;
    float GetFar() const;
    float GetAspect() const;

    DirectX::XMVECTOR GetForwardVector() const;
    DirectX::XMVECTOR GetRightVector() const;
    DirectX::XMVECTOR GetUpVector() const;

private:
    void UpdateProjection(); 

private:
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_rotation; 
    DirectX::XMFLOAT3 m_up;

    std::unique_ptr<Frustum> m_frustum;

    DirectX::XMMATRIX m_viewMatrix;
    DirectX::XMMATRIX m_projectionMatrix;

    float m_fov, m_near, m_far, m_aspect;
    float m_maxPitch, m_minPitch;
    float m_maxFov, m_minFov;
}; // Camera