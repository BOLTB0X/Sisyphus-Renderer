#pragma once
#include <iostream>
#include <directxmath.h>

namespace SharedConstants {

    namespace CameraConstants {
        static constexpr float MAX_PITCH = 89.0f;
        static constexpr float MIN_PITCH = -89.0f;

        static constexpr float MIN_FOV = 1.0f;
        static constexpr float MAX_FOV = 179.0f;


        static constexpr DirectX::XMFLOAT3 DEFAULT_POSITION = { 0.0f, 0.0f, -5.0f };
        static constexpr DirectX::XMFLOAT3 DEFAULT_ROTATION = { 0.0f, 0.0f, 0.0f };
        static constexpr float DEFAULT_FOV = 60.0f;

    } // CameraConstants
} // SharedConstants