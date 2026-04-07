#pragma once
#include <iostream>
#include <string>

namespace SharedConstants {

    namespace PathConstants {
        const std::wstring COLOR_VS = L"HLSL/ColorVS.hlsl";
        const std::wstring COLOR_PS = L"HLSL/ColorPS.hlsl";

        const std::wstring STONE_VS = L"HLSL/StoneVS.hlsl";
        const std::wstring STONE_PS = L"HLSL/StonePS.hlsl";

        const std::wstring NOISEGEN_CS = L"HLSL/NoiseGeneratorCS.hlsl";
        const std::wstring BASE_NOISE_CS = L"HLSL/BaseNoiseCS.hlsl";
        const std::wstring DETAIL_NOISE_CS = L"HLSL/DetailNoiseCS.hlsl";
        const std::wstring SLICER_CS = L"HLSL/VolumeSlicerCS.hlsl";
        const std::wstring WMAP_CS = L"HLSL/WeatherMapCS.hlsl";

        const std::wstring ATMOSPHERE_VS = L"HLSL/AtmosphereVS.hlsl";
        const std::wstring ATMOSPHERE_PS = L"HLSL/AtmospherePS.hlsl";

        const std::wstring SKYBOX_VS = L"HLSL/SkyboxVS.hlsl";
        const std::wstring SKYBOX_PS = L"HLSL/SkyboxPS.hlsl";

        const std::wstring GROUND_VS = L"HLSL/GroundVS.hlsl";
        const std::wstring GROUND_PS = L"HLSL/GroundPS.hlsl";

        const std::wstring DEPTH_VS = L"HLSL/DepthVS.hlsl";
        const std::wstring COMPOSITE_VS = L"HLSL/CompositeVS.hlsl";
        const std::wstring COMPOSITE_PS = L"HLSL/CompositePS.hlsl";

        const std::string STONE = "assets/Stone/Stone.fbx";
        const std::string KEY_CLOUD_VOL = "CloudNoise";
        const std::string KEY_BASE_VOL = "BaseNoise";
        const std::string KEY_DETAIL_VOL = "DetailNoise";
    } // PathConstants
}