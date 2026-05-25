#pragma once
#include <iostream>
#include <string>
#include <vector>

namespace SharedConstants {
    namespace ShadowConstants {
        static constexpr float VIEW_WIDTH = 500.0f;  // 빛이 비추는 가로 범위
        static constexpr float VIEW_HEIGHT = 500.0f;  // 빛이 비추는 세로 범위
        static constexpr float NEAR_Z = 0.1f;
        static constexpr float FAR_Z = 300.0f;
        static constexpr float SHADOWMAP_WIDTH = 2048;
        static constexpr float SHADOWMAP_HEIGHT = 2048;
    } // ShadowConstants
}