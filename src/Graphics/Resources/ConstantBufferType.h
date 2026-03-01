#pragma once
#include <directxmath.h>

namespace ConstantBuffer {
    struct MatrixBuffer {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    }; // MatrixBuffer

    struct CameraBuffer {
        // Row 1
        DirectX::XMMATRIX world;
        // Row 2
        DirectX::XMMATRIX view;
        // Row 3
        DirectX::XMMATRIX projection;
        // Row 4
        DirectX::XMFLOAT3 cameraPosition;
        float             padding;
    }; // CameraBuffer

    struct LightBuffer {
        // Row 1
        DirectX::XMFLOAT4 diffuseColor;
        // Row 2
        DirectX::XMFLOAT3 lightDirection;
        float padding;
    }; // LightBuffer
} // ConstantBuffer