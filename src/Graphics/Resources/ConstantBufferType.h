#pragma once
#include <directxmath.h>

namespace ConstantBuffer {
    struct MatrixBuffer {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;

        MatrixBuffer() 
            : world(DirectX::XMMatrixIdentity()),
              view(DirectX::XMMatrixIdentity()),
              projection(DirectX::XMMatrixIdentity()) {
		}
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

		CameraBuffer() 
            : world(DirectX::XMMatrixIdentity()), view(DirectX::XMMatrixIdentity()),
              projection(DirectX::XMMatrixIdentity()), 
              cameraPosition(0.0f, 0.0f, 0.0f), padding(0.0f) {
        }

        CameraBuffer(DirectX::XMMATRIX w, DirectX::XMMATRIX v, DirectX::XMMATRIX p, DirectX::XMFLOAT3 camPos)
            : world(w), view(v), projection(p), cameraPosition(camPos), padding(0.0f) {
		}
    }; // CameraBuffer

    struct LightBuffer {
        // Row 1
        DirectX::XMFLOAT4 diffuseColor;
        // Row 2
        DirectX::XMFLOAT3 lightDirection;
        float padding;

        LightBuffer() 
            : diffuseColor(1.0f, 1.0f, 1.0f, 1.0f),
              lightDirection(0.0f, -1.0f, 0.0f),
              padding(0.0f) {
		}

        LightBuffer(DirectX::XMFLOAT4 diffuse, DirectX::XMFLOAT3 dir)
            : diffuseColor(diffuse), lightDirection(dir), padding(0.0f) {
        }
    }; // LightBuffer
} // ConstantBuffer