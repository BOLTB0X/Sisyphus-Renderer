#pragma once
#include <iostream>
#include <directxmath.h>

namespace SharedConstants {

    namespace CameraConstants {
        static constexpr float MAX_PITCH = 89.0f;
        static constexpr float MIN_PITCH = -89.0f;

        static constexpr float MIN_FOV = 1.0f;
        static constexpr float MAX_FOV = 120.0f;

        static constexpr DirectX::XMFLOAT3 DEFAULT_POSITION = { 0.0f, 0.0f, -5.0f };
        static constexpr DirectX::XMFLOAT3 DEFAULT_ROTATION = { 0.0f, 0.0f, 0.0f };

        static constexpr float DEFAULT_POS_X = 0.0f;
        static constexpr float DEFAULT_POS_Y = 0.0f;
        static constexpr float DEFAULT_POS_Z = -5.0f;

        static constexpr float DEFAULT_ROT_X = 0.0f;
        static constexpr float DEFAULT_ROT_Y = 0.0f;
        static constexpr float DEFAULT_ROT_Z = 0.0f;

        static constexpr float DEFAULT_FOV = 45.0f;
    } // CameraConstants
} // SharedConstants