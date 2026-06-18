#pragma once
#include <iostream>
#include <string>

namespace SharedConstants {

    namespace PathConstants {
        static const std::wstring COLOR_VS = L"HLSL/ColorVS.hlsl";
        static const std::wstring COLOR_PS = L"HLSL/ColorPS.hlsl";

        static const std::wstring PBR_VS = L"HLSL/PBRModelVS.hlsl";
        static const std::wstring STONE_PS = L"HLSL/StonePS.hlsl";
        static const std::wstring TREE_PS = L"HLSL/TreePS.hlsl";
        static const std::wstring STONE_PILLAR_PS = L"HLSL/StonePillarPS.hlsl";
        static const std::wstring ARCA_PS = L"HLSL/ArcaPS.hlsl";
        static const std::wstring IDOL_PS = L"HLSL/IdolPS.hlsl";

        static const std::wstring NOISEGEN_CS = L"HLSL/NoiseGeneratorCS.hlsl";
        static const std::wstring CLOUD_MAP_CS = L"HLSL/CloudMapCS.hlsl";
        static const std::wstring WORLEY_NOISE_CS = L"HLSL/WorleyNoiseCS.hlsl";
        static const std::wstring PERLINE_WORLEY_CS = L"HLSL/PerlinWorleyCS.hlsl";

        static const std::wstring ATMOSPHERE_CS = L"HLSL/AtmosphereMapCS.hlsl";
        static const std::wstring VOLUMETRIC_CLOUD_CS = L"HLSL/VolumetricCloudCS.hlsl";

        static const std::wstring SKYBOX_VS = L"HLSL/SkyboxVS.hlsl";
        static const std::wstring SKYBOX_PS = L"HLSL/SkyboxPS.hlsl";

        static const std::wstring GROUND_VS = L"HLSL/GroundVS.hlsl";
        static const std::wstring GROUND_PS = L"HLSL/GroundPS.hlsl";
        //static const std::wstring WATER_VS = L"HLSL/WaterVS.hlsl";
        static const std::wstring WATER_PS = L"HLSL/WaterCompositePS.hlsl";

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

        static const std::wstring GRASS_FAR_VS = L"HLSL/GrassFarVS.hlsl";
        static const std::wstring GRASS_FAR_PS = L"HLSL/GrassFarPS.hlsl";

        static const std::wstring TRANSPARENT_DEPTH_VS = L"HLSL/TransparentDepthVS.hlsl";
        static const std::wstring TRANSPARENT_DEPTH_PS = L"HLSL/TransparentDepthPS.hlsl";

        static const std::wstring SKINNED_VS = L"HLSL/SkinnedVS.hlsl";
        static const std::wstring SKINNED_DEPTH_VS = L"HLSL/SkinnedDepthVS.hlsl";
        static const std::wstring RAKSHASA_PS = L"HLSL/RakshasaPS.hlsl";
        static const std::wstring RIGID_SAMPLE_PS = L"HLSL/RigidSampePS.hlsl";

        static const std::string STONE = "assets/Stone/Stone.fbx";
        static const std::string TREE = "assets/Tree/TreeGen.fbx";
        static const std::string STONE_PILLAR = "assets/StonePillar/StonePillar_low.fbx";
        static const std::string ARCA = "assets/Arca/Arca.fbx";
        static const std::string RAKSHASA = "assets/Rakshasa/Rakshasa.fbx";
        static const std::string STEVE = "assets/RigidSample/RigidSample.fbx";

        static const std::string BLUE_NOISE = "assets/Noise/LDR_LLL1_0.png";
        static const std::string NOISE_2D = "assets/Noise/noise.png";
        static const std::string HEIGHT = "assets/Map/heightmap.png";
        static const std::string GRASS = "assets/Grass/grass.dds";
        //static const std::string FARAWAY_GRASS = "assets/Ground/farawayGrass.tga";
        static const std::string GROUND_COL = "assets/Ground/Ground_baseColor.tga";
        static const std::string GROUND_NOR = "assets/Ground/Ground_normal.tga";
        static const std::string WATER_NOR = "assets/Map/water_normal.jpg";
        static const std::string WATER_WAVE_NOR = "assets/Map/water_wave_normal.jpg";
        static const std::string FLOW_MAP = "assets/Map/flowmap.png";

        static const std::string KEY_CLOUD_VOL = "CloudNoise";
        static const std::string KEY_PERLIN_NOISE = "PerlinNoise";
        static const std::string KEY_WORLEY_NOISE = "WorleyNoise";
        static const std::string KEY_SCENE_RT = "Scene";
        static const std::string KEY_REFLECTION_RT = "Reflection";
    } // PathConstants
}