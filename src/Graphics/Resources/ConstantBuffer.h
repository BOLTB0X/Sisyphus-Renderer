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

    struct WorldBuffer {
        DirectX::XMMATRIX world;

        WorldBuffer() {
            world = DirectX::XMMatrixIdentity();
        }
    }; // WorldBuffer;

    struct FrameBuffer {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMMATRIX viewInv;
        DirectX::XMMATRIX projInv;
        DirectX::XMFLOAT3 cameraPosition;
        float             cameraFov;
        DirectX::XMFLOAT2 screenResolution;
        float             time;
        float             padding2;

        FrameBuffer() :
            view(DirectX::XMMatrixIdentity()),
            projection(DirectX::XMMatrixIdentity()),
            viewInv(DirectX::XMMatrixIdentity()),
            projInv(DirectX::XMMatrixIdentity()),
            cameraPosition(0.0f, 0.0f, 0.0f), cameraFov(0.0f),
            screenResolution((float)SharedConstants::ScreenConstants::WIDTH, (float)SharedConstants::ScreenConstants::HEIGHT),
            time(0.0f), padding2(0.0f) {
		}
    }; // FrameBuffer

    struct DirectionalLightBuffer {
        DirectX::XMFLOAT3 direction;
        float             padding1;

        DirectX::XMFLOAT4 ambient;

        DirectX::XMFLOAT4 diffuse;

        DirectX::XMFLOAT4 sunset;

        DirectX::XMFLOAT4 night;

        DirectX::XMFLOAT3 lookAt;
        float             padding2;

        DirectX::XMMATRIX lightViewMatrix;

        DirectX::XMMATRIX lightProjectionMatrix;

        DirectX::XMMATRIX objectViewMatrix;

        DirectX::XMMATRIX objectProjectionMatrix;

        float             shadowMapWidth;
        float             shadowMapHeight;
        float             shadowBias;
        float             shadowSpread;
        DirectX::XMFLOAT4 padding3;

        DirectionalLightBuffer() :
            direction(0.0f, -1.0f, 0.0f), padding1(0.0f),
            ambient(0.2f, 0.2f, 0.2f, 1.0f),
            diffuse(1.0f, 1.0f, 1.0f, 1.0f),
            sunset(0.0f, 0.0f, 0.0f, 1.0f),
            night(0.0f, 0.0f, 0.0f, 1.0f),
            lookAt(0.0f, 0.0f, 0.0f), padding2(0.0f),
            lightViewMatrix(DirectX::XMMatrixIdentity()),
            lightProjectionMatrix(DirectX::XMMatrixIdentity()),
            objectViewMatrix(DirectX::XMMatrixIdentity()),
            objectProjectionMatrix(DirectX::XMMatrixIdentity()), shadowMapWidth(0.0f),
            shadowMapHeight(0.0f), shadowBias(0.0f), shadowSpread(0.0f),
            padding3(0.0f, 0.0f, 0.0f, 0.0f) {
        }
    }; // DirectionalLightBuffer

    struct ResolutionBuffer {
        DirectX::XMFLOAT2 resolution;
        DirectX::XMFLOAT2 padding;

        ResolutionBuffer() : resolution(1024.0f, 512.0f), padding(0.0f, 0.0f) {
        }
    }; // ResolutionBuffer

    struct ClipPlaneBuffer {
        DirectX::XMFLOAT4 clipPlane;

        ClipPlaneBuffer() {
            clipPlane = { 0.0f, 0.0f, 0.0f, 0.0f };
        }
    }; // ClipPlaneBuffer

    struct ReflectionMatrixBuffer {
        DirectX::XMFLOAT4X4 reflectView;

        ReflectionMatrixBuffer() {
            reflectView = DirectX::XMFLOAT4X4();
        }
    }; // ReflectionMatrixBuffer

    struct TessellationControlBuffer {
        float             minTessDist; // 이 거리 안쪽은 가장 세밀하게
        float             maxTessDist; // 이 거리 바깥은 쪼개지 않음 (Tess=1)
        float             minTessFactor; // 최소 분할 (원래 폴리곤)
        float             maxTessFactor; // 최대 분할 (하드웨어 한계 64)
        DirectX::XMFLOAT4 padding;

        TessellationControlBuffer() : minTessDist(20.0f), maxTessDist(500.0f),
            minTessFactor(1.0f), maxTessFactor(64.0f), padding(0.0f, 0.0f, 0.0f, 0.0f){
        }
    }; // TessellationControlBuffer


    struct PlacementBuffer {
        DirectX::XMFLOAT3 cameraPos;
        float             waterLevel;

        float             terrainWidth;
        float             terrainDepth;
        float             grassDensity;
        float             treeDensity;

        float             dist;
        float             heightScale;
		DirectX::XMFLOAT2 padding;
    }; // PlacementBuffer

    struct DrawInstancedIndirectArgs {
        UINT VertexCountPerInstance;
        UINT InstanceCount;
        UINT StartVertexLocation;
        UINT StartInstanceLocation;
    }; // DrawInstancedIndirectArgs

    struct DrawIndexedInstancedIndirectArgs {
        UINT IndexCountPerInstance;
        UINT InstanceCount;
        UINT StartIndexLocation;
        INT  BaseVertexLocation;
        UINT StartInstanceLocation;
    }; // DrawIndexedInstancedIndirectArgs

} // ConstantBuffer