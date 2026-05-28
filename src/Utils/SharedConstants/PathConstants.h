#pragma once
#include <iostream>
#include <string>

namespace SharedConstants {

    namespace PathConstants {
        static const std::wstring COLOR_VS = L"HLSL/ColorVS.hlsl";
        static const std::wstring COLOR_PS = L"HLSL/ColorPS.hlsl";

        static const std::wstring STONE_VS = L"HLSL/StoneVS.hlsl";
        static const std::wstring STONE_PS = L"HLSL/StonePS.hlsl";
        static const std::wstring NOISEGEN_CS = L"HLSL/NoiseGeneratorCS.hlsl";
        static const std::wstring CLOUD_MAP_CS = L"HLSL/CloudMapCS.hlsl";
        static const std::wstring WORLEY_NOISE_CS = L"HLSL/WorleyNoiseCS.hlsl";

        static const std::wstring ATMOSPHERE_CS = L"HLSL/AtmosphereMapCS.hlsl";
        static const std::wstring VOLUMETRIC_CLOUD_CS = L"HLSL/VolumetricCloudCS.hlsl";

        static const std::wstring SKYBOX_VS = L"HLSL/SkyboxVS.hlsl";
        static const std::wstring SKYBOX_PS = L"HLSL/SkyboxPS.hlsl";

        static const std::wstring GROUND_VS = L"HLSL/GroundVS.hlsl";
        static const std::wstring GROUND_PS = L"HLSL/GroundPS.hlsl";

        static const std::wstring DEPTH_VS = L"HLSL/DepthVS.hlsl";
        static const std::wstring POST_VS = L"HLSL/PostVS.hlsl";
        static const std::wstring CLOUD_COMPOSITE_PS = L"HLSL/CloudCompositePS.hlsl";
        static const std::wstring BLOOM_PS = L"HLSL/BloomPS.hlsl";
        static const std::wstring GOD_RAY_PS = L"HLSL/GodRayPS.hlsl";
        static const std::wstring LENS_FLARE_PS = L"HLSL/LensflarePS.hlsl";
        static const std::wstring COMPOSITE_PS = L"HLSL/CompositePS.hlsl";
        static const std::wstring TAA_PS = L"HLSL/TemporalAntiAliasingPS.hlsl";
        static const std::wstring GRASS_VS = L"HLSL/GrassVS.hlsl";
        static const std::wstring GRASS_GS = L"HLSL/GrassGS.hlsl";
        static const std::wstring GRASS_PS = L"HLSL/GrassPS.hlsl";

        static const std::string STONE = "assets/Stone/Stone.fbx";
        static const std::string BLUE_NOISE = "assets/Noise/LDR_LLL1_0.png";
        static const std::string NOISE_2D = "assets/Noise/noise.png";
        static const std::string HEIGHT = "assets/Map/heightmap.png";

        static const std::string KEY_CLOUD_VOL = "CloudNoise";
        static const std::string KEY_WORLEY_NOISE = "WorleyNoise";
    } // PathConstants
}