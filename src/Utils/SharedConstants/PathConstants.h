#pragma once
#include <iostream>
#include <string>

namespace SharedConstants {

    namespace PathConstants {
        const std::wstring COLOR_VS = L"HLSL/ColorVS.hlsl";
        const std::wstring COLOR_PS = L"HLSL/ColorPS.hlsl";

        const std::wstring STONE_VS = L"HLSL/StoneVS.hlsl";
        const std::wstring STONE_PS = L"HLSL/StonePS.hlsl";

        const std::string STONE = "assets/Stone/Stone.fbx";
    } // PathConstants
}