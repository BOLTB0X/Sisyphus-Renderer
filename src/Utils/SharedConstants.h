// SharedConstants.h
#pragma once
#include <iostream>
#include <string>

namespace SharedConstants {

    namespace DefaultScreen {
        static constexpr bool  FULL_SCREEN    = false;
        static constexpr bool  VSYNC_ENABLED  = true;
        static constexpr int   WIDTH          = 800;
        static constexpr int   HEIGHT         = 600;
        static constexpr float DEPTH          = 1000.0f;
        static constexpr float NEAR_PLANE     = 0.1f;
    } // DefaultScreen

    namespace CameraSettings {
        static constexpr float MAX_PITCH = 89.0f;
        static constexpr float MIN_PITCH = -89.0f;

        static constexpr float MIN_FOV = 1.0f;
        static constexpr float MAX_FOV = 120.0f;
    } // CameraSettings

    namespace WidgetName {
        inline const std::string PERFORMANCE = "Performance";
    } // WidgetName

    namespace HLSL {
        const std::wstring COLOR_VS = L"HLSL/ColorVS.hlsl";
        const std::wstring COLOR_PS = L"HLSL/ColorPS.hlsl";
    } // HLSL
    
} // SharedConstants