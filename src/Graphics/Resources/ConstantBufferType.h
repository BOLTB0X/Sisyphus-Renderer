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

    struct NoiseBuffer {
        // Row 1
        DirectX::XMFLOAT3 textureSize;
        float             perlinFreq;
		// Row 2
        float             worleyFreq;
        float             detailFreqG;
        float             detailFreqB;
        float             detailFreqA;
        // Row 3
        int               octaves;
        float             remapBias;
        DirectX::XMFLOAT2 padding;

        NoiseBuffer()
            : textureSize(1.0f, 1.0f, 1.0f), perlinFreq(1.0f),
              worleyFreq(1.0f), detailFreqG(1.0f), detailFreqB(1.0f), detailFreqA(1.0f),
               octaves(1), remapBias(0.0f), padding(0.0f, 0.0f) {
        }

        NoiseBuffer(DirectX::XMFLOAT3 texSize, float perlinF,
            float worleyF, float detailFG, float detailFB, float detailFA,
            int octs, float remapB)
            : textureSize(texSize), perlinFreq(perlinF),
            worleyFreq(worleyF), detailFreqG(detailFG), detailFreqB(detailFB), detailFreqA(detailFA),
              octaves(octs), remapBias(remapB), padding(0.0f, 0.0f) {
		}
    }; // NoiseBuffer

    //struct CloudBuffer {
    //    DirectX::XMMATRIX invView;
    //    DirectX::XMMATRIX invProjection;
    //    DirectX::XMFLOAT3 camPos;
    //    float             padding;

    //    CloudBuffer() : invView(DirectX::XMMatrixIdentity()),
    //        invProjection(DirectX::XMMatrixIdentity()),
    //        camPos(0.0f, 0.0f, 0.0f), padding(0.0f) {
    //    }
    //}; // CloudBuffer
    
} // ConstantBuffer