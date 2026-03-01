#pragma once
#include <directxmath.h>

namespace VertexTypes {
    struct ColorVertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    }; // ColorVertex

    struct FBRVertex {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT2 texture;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT3 tangent;
        DirectX::XMFLOAT3 binormal;
    }; // FBRVertex
} // VertexTypes