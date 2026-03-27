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

    struct MatCameraBuffer {
        // Row 1
        DirectX::XMMATRIX world;
        // Row 2
        DirectX::XMMATRIX view;
        // Row 3
        DirectX::XMMATRIX projection;
        // Row 4
        DirectX::XMFLOAT3 cameraPosition;
        float             padding;

		MatCameraBuffer() 
            : world(DirectX::XMMatrixIdentity()), view(DirectX::XMMatrixIdentity()),
              projection(DirectX::XMMatrixIdentity()), 
              cameraPosition(0.0f, 0.0f, 0.0f), padding(0.0f) {
        }

        MatCameraBuffer(DirectX::XMMATRIX w, DirectX::XMMATRIX v, DirectX::XMMATRIX p, DirectX::XMFLOAT3 camPos)
            : world(w), view(v), projection(p), cameraPosition(camPos), padding(0.0f) {
		}
    }; // MatCameraBuffer

    struct CommonBuffer {
        // Row 1
        DirectX::XMMATRIX world;
        // Row 2
        DirectX::XMMATRIX view;
        // Row 3
        DirectX::XMMATRIX projection;
        // Row 4
        DirectX::XMFLOAT3 cameraPosition;
        float             padding1;
        // Row 5
        DirectX::XMMATRIX viewInv;
        // Row 6
        DirectX::XMMATRIX projInv;
        // Row 7
        DirectX::XMFLOAT3 lightDirection;
        float             padding2;
        // Row 8
        DirectX::XMFLOAT4 lightDiffuse;
        // Row 9
        DirectX::XMFLOAT2 resolution;

        CommonBuffer() :
            world(DirectX::XMMatrixIdentity()),
            view(DirectX::XMMatrixIdentity()),
            projection(DirectX::XMMatrixIdentity()),
            cameraPosition(0.0f, 0.0f, 0.0f), padding1(0.0f),
            viewInv(DirectX::XMMatrixIdentity()),
            projInv(DirectX::XMMatrixIdentity()),
            lightDirection(SharedConstants::BuffersConstants::LIGHT_DIR), padding2(0.0f),
            lightDiffuse(SharedConstants::BuffersConstants::LIGHT_DIFFUSE),
            resolution((float)SharedConstants::ScreenConstants::WIDTH, (float)SharedConstants::ScreenConstants::HEIGHT) {
        }
    }; // CommonBuffer

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

    struct ShadowBuffer {
        // Row 1
        DirectX::XMMATRIX world;
        // Row 2
        DirectX::XMMATRIX lightView;
        // Row 3
        DirectX::XMMATRIX lightProjection;
        // Row 4
        float             mapWidth;
        float             mapHeight;
        float             bias;
        float             spread;
        // Row 5
        DirectX::XMFLOAT4 padding;

        ShadowBuffer() {
            world = DirectX::XMMatrixIdentity();
            lightView = DirectX::XMMatrixIdentity();
            lightProjection = DirectX::XMMatrixIdentity();
            mapWidth = 0.0f;
            mapHeight = 0.0f;
            bias = 0.0f;
            spread = 0.0f;
            padding = { 0.0f, 0.0f, 0.0f, 0.0f };
        }
    }; // ShadowBuffer

    struct CameraBuffer {
        DirectX::XMFLOAT3 cameraPosition;
        float padding;

        DirectX::XMMATRIX viewInv;
        DirectX::XMMATRIX projInv;

        CameraBuffer() :
            cameraPosition(0.0f, 0.0f, 0.0f), padding(0.0f) {
            viewInv = DirectX::XMMatrixIdentity();
            projInv = DirectX::XMMatrixIdentity();
        }

    }; // CameraBuffer

} // ConstantBuffer