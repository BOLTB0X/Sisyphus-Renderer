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
        static const std::wstring PERLINE_CS = L"HLSL/PerlinHeightMapCS.hlsl";

        static const std::wstring ATMOSPHERE_CS = L"HLSL/AtmosphereMapCS.hlsl";
        static const std::wstring VOLUMETRIC_CLOUD_CS = L"HLSL/VolumetricCloudCS.hlsl";

        static const std::wstring SKYBOX_VS = L"HLSL/SkyboxVS.hlsl";
        static const std::wstring SKYBOX_PS = L"HLSL/SkyboxPS.hlsl";

        static const std::wstring GROUND_VS = L"HLSL/GroundVS.hlsl";
        static const std::wstring GROUND_PS = L"HLSL/GroundPS.hlsl";
        static const std::wstring WATER_POST_PS = L"HLSL/WaterCompositePS.hlsl";
        static const std::wstring WATER_VS = L"HLSL/WaterVS.hlsl";
        static const std::wstring WATER_PS = L"HLSL/WaterPS.hlsl";

        static const std::wstring DEPTH_VS = L"HLSL/DepthVS.hlsl";
        static const std::wstring POST_VS = L"HLSL/PostVS.hlsl";
        static const std::wstring CLOUD_COMPOSITE_PS = L"HLSL/CloudCompositePS.hlsl";
        static const std::wstring BLOOM_PS = L"HLSL/BloomPS.hlsl";
        static const std::wstring GOD_RAY_PS = L"HLSL/GodRayPS.hlsl";
        static const std::wstring LENS_FLARE_PS = L"HLSL/LensflarePS.hlsl";
        static const std::wstring COMPOSITE_PS = L"HLSL/CompositePS.hlsl";
        static const std::wstring TAA_PS = L"HLSL/TemporalAntiAliasingPS.hlsl";
        static const std::wstring QUAD_TREE_GRASS_VS = L"HLSL/QuadTreeGrassVS.hlsl";
        static const std::wstring GRASS_GS = L"HLSL/GrassGS.hlsl";
        static const std::wstring GRASS_PS = L"HLSL/GrassPS.hlsl";

        static const std::wstring GRASS_PLACEMENT_CS = L"HLSL/PlacementSeedCS.hlsl";
        static const std::wstring QUAD_TREE_GRASS_FAR_VS = L"HLSL/QuadTreeGrassFarVS.hlsl";
        static const std::wstring GRASS_FAR_PS = L"HLSL/GrassFarPS.hlsl";
        static const std::wstring PLACE_GRASS_VS = L"HLSL/PlaceGrassVS.hlsl";
        static const std::wstring PLACE_GRASS_FAR_VS = L"HLSL/PlaceGrassFarVS.hlsl";

        static const std::wstring TRANSPARENT_DEPTH_VS = L"HLSL/TransparentDepthVS.hlsl";
        static const std::wstring TRANSPARENT_DEPTH_PS = L"HLSL/TransparentDepthPS.hlsl";

        static const std::wstring SKINNED_VS = L"HLSL/SkinnedVS.hlsl";
        static const std::wstring SKINNED_DEPTH_VS = L"HLSL/SkinnedDepthVS.hlsl";
        static const std::wstring RAKSHASA_PS = L"HLSL/RakshasaPS.hlsl";
        static const std::wstring RIGID_SAMPLE_PS = L"HLSL/RigidSampePS.hlsl";

        static const std::wstring TERRAIN_VS = L"HLSL/TerrainVS.hlsl";
        static const std::wstring TERRAIN_PS = L"HLSL/TerrainPS.hlsl";
        static const std::wstring TERRAIN_HS = L"HLSL/TerrainHS.hlsl";
        static const std::wstring TERRAIN_DS = L"HLSL/TerrainDS.hlsl";

        static const std::wstring INSTANCED_VS = L"HLSL/InstancedVS.hlsl";
        static const std::wstring INSTANCED_DEPTH_VS = L"HLSL/InstancedDepthVS.hlsl";
        static const std::wstring INSTANCED_TRANSPARENT_DEPTH_VS = L"HLSL/InstancedTransparentDepthVS.hlsl";
        static const std::wstring TERRAIN_DEPTH_DS = L"HLSL/TerrainDepthDS.hlsl";

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
        static const std::string GROUND_COL = "assets/Ground/Ground_baseColor.tga";
        static const std::string GROUND_NOR = "assets/Ground/Ground_normal.tga";
        static const std::string WATER_NOR = "assets/Map/water_normal.jpg";
        static const std::string WATER_WAVE_NOR = "assets/Map/water_wave_normal.jpg";
        static const std::string FLOW_MAP = "assets/Map/flowmap.png";
        static const std::string TERRAIN_COL = "assets/Terrain/Terrain_Col.jpg";
        static const std::string TERRAIN_GRASS = "assets/Terrain/Terrain_GRASS.jpg";
        static const std::string TERRAIN_RDIFF = "assets/Terrain/Terrain_rdiffuse.jpg";
        static const std::string TERRAIN_RNOL = "assets/Terrain/Terrain_rnormal.jpg";
        static const std::string TERRAIN_SAND = "assets/Terrain/Terrain_Sand.jpg";
        static const std::string TERRAIN_SNOW = "assets/Terrain/Terrain_snow.jpg";

        static const std::string KEY_CLOUD_VOL = "CloudNoise";
        static const std::string KEY_PERLIN_NOISE = "PerlinNoise";
        static const std::string KEY_WORLEY_NOISE = "WorleyNoise";
        static const std::string KEY_SCENE_RT = "Scene";
        static const std::string KEY_REFLECTION_RT = "Reflection";
        static const std::string KEY_REFRACTION_RT = "Refraction";
        static const std::string KEY_NORMAL_RT = "NormalRT";
    } // PathConstants
}