#pragma once
#include <directxmath.h>
#include "Utils/SharedConstants/BuffersConstants.h"
#include "Utils/SharedConstants/ScreenConstants.h"

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

   
    struct ShadowBuffer {
        // Row 1
        DirectX::XMMATRIX world;
        float             mapWidth;
        float             mapHeight;
        float             bias;
        float             spread;
        DirectX::XMFLOAT4 padding;

        ShadowBuffer() {
            world = DirectX::XMMatrixIdentity();
            mapWidth = 0.0f;
            mapHeight = 0.0f;
            bias = 0.0f;
            spread = 0.0f;
            padding = { 0.0f, 0.0f, 0.0f, 0.0f };
        }
    }; // ShadowBuffer

    struct FrameBuffer {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMMATRIX viewInv;
        DirectX::XMMATRIX projInv;
        DirectX::XMFLOAT3 cameraPosition;
        float             padding1;
        DirectX::XMFLOAT2 screenResolution;
        float             time;
        float             padding2;

        FrameBuffer() :
            view(DirectX::XMMatrixIdentity()),
            projection(DirectX::XMMatrixIdentity()),
            viewInv(DirectX::XMMatrixIdentity()),
            projInv(DirectX::XMMatrixIdentity()),
            cameraPosition(0.0f, 0.0f, 0.0f), padding1(0.0f),
            screenResolution((float)SharedConstants::ScreenConstants::WIDTH, (float)SharedConstants::ScreenConstants::HEIGHT),
            time(0.0f), padding2(0.0f) {
		}
    }; // FrameBuffer

    struct DirectionalLightBuffer {
        DirectX::XMFLOAT3 direction;
        float             padding1;
        DirectX::XMFLOAT4 ambient;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT3 lookAt;
        float             padding2;
        DirectX::XMMATRIX lightViewMatrix;
        DirectX::XMMATRIX lightProjectionMatrix;

        DirectionalLightBuffer() :
            direction(0.0f, -1.0f, 0.0f), padding1(0.0f),
            ambient(0.2f, 0.2f, 0.2f, 1.0f),
            diffuse(1.0f, 1.0f, 1.0f, 1.0f),
            lookAt(0.0f, 0.0f, 0.0f), padding2(0.0f),
            lightViewMatrix(DirectX::XMMatrixIdentity()),
            lightProjectionMatrix(DirectX::XMMatrixIdentity()) {
        }
    }; // DirectionalLightBuffer

} // ConstantBuffer