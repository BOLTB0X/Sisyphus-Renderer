// SharedConstants.h
#pragma once
#include <iostream>
#include <string>
#include <vector>

namespace SharedConstants {

    namespace ScreenConstants {
        static constexpr bool  FULL_SCREEN    = false;
        static constexpr bool  VSYNC_ENABLED  = true;
        static constexpr int   WIDTH          = 800;
        static constexpr int   HEIGHT         = 600;
        static constexpr float DEPTH          = 1000.0f;
        static constexpr float NEAR_PLANE     = 0.1f;
    } // ScreenConstants

} // SharedConstants