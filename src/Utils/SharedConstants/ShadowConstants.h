#pragma once
#include <iostream>
#include <string>
#include <vector>

namespace SharedConstants {
    namespace ShadowConstants {
		static constexpr float OBJECT_VIEW_WIDTH = 150.0f;
		static constexpr float OBJECT_VIEW_HEIGHT = 150.0f;
        static constexpr float TERRAIN_VIEW_WIDTH = 2000.0f;
        static constexpr float TERRAIN_VIEW_HEIGHT = 2000.0f;
        static constexpr float NEAR_Z = 0.1f;
        static constexpr float FAR_Z = 2000.0f;
        static constexpr float OBJECT_FAR_Z = 300.0f;
        static constexpr float SHADOWMAP_WIDTH = 2048;
        static constexpr float SHADOWMAP_HEIGHT = 2048;
    } // ShadowConstants
}