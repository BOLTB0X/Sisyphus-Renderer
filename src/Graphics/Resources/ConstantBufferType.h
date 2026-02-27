#pragma once
#include <directxmath.h>

namespace ConstantBuffer {
    struct MatrixBuffer {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    };
} // ConstantBuffer