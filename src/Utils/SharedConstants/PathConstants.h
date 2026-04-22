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
        const std::wstring CLOUD_MAP_CS = L"HLSL/CloudMapCS.hlsl";
        const std::wstring WORLEY_NOISE_CS = L"HLSL/WorleyNoiseCS.hlsl";

        const std::wstring ATMOSPHERE_CS = L"HLSL/AtmosphereCS.hlsl";
        const std::wstring VOLUMETRIC_CLOUD_CS = L"HLSL/VolumetricCloudCS.hlsl";

        const std::wstring SKYBOX_VS = L"HLSL/SkyboxVS.hlsl";
        const std::wstring SKYBOX_PS = L"HLSL/SkyboxPS.hlsl";

        const std::wstring GROUND_VS = L"HLSL/GroundVS.hlsl";
        const std::wstring GROUND_PS = L"HLSL/GroundPS.hlsl";

        const std::wstring DEPTH_VS = L"HLSL/DepthVS.hlsl";
        const std::wstring POST_VS = L"HLSL/PostVS.hlsl";
        const std::wstring CLOUD_COMPOSITE_PS = L"HLSL/CloudCompositePS.hlsl";
        const std::wstring BLOOM_PS = L"HLSL/BloomPS.hlsl";
        const std::wstring TAA_PS = L"HLSL/TemporalAntiAliasingPS.hlsl";

        const std::string STONE = "assets/Stone/Stone.fbx";
        const std::string BLUE_NOISE = "assets/Noise/LDR_LLL1_0.png";
        const std::string KEY_CLOUD_VOL = "CloudNoise";
        const std::string KEY_WORLEY_NOISE = "WorleyNoise";
    } // PathConstants
}