#pragma once
#include <iostream>
#include <directxmath.h>
#include "Utils/SharedConstants/ScreenConstants.h"

namespace SharedConstants {

    namespace BuffersConstants {

        // Atmoshpere
        static constexpr float             PLANET_RADIUS = 6371000.0f;
        static constexpr float             ATMOSPHERE_RADIUS = 6471000.0f;
        static constexpr float             ATMOSPHERE_HEIGHT = 100000.0f;
        static constexpr float             RAYLEIGH_HEIGHT = 8000.0f;
        static constexpr float             MIE_HEIGHT = 1.2e3f;
        static constexpr float             ABSORPTION_HEIGHT = 30000.0f;
        static constexpr float             ABSORPTION_FALLOFF = 4000.0f;

        static constexpr DirectX::XMFLOAT3 RAYLEIGH_SCATTERING_COEFFICIENT = { 5.8e-6f, 13.5e-6f, 33.1e-6f };
        static constexpr float             MIE_BETA = 21e-6f;
        static constexpr DirectX::XMFLOAT3 MIE_SCATTERING_COEFFICIENT = { 21e-6f, 21e-6f, 21e-6f };

        static constexpr float             AMBIENT_BETA = 0.0f;
        static constexpr float             G = 0.9f;
        static constexpr float             ATMOSPHERE_INTENWSITY = 40.0f;

        static constexpr DirectX::XMFLOAT4 ZENITH_COLOR = { 0.0f, 0.2f, 0.6f, 1.0f };
        static constexpr DirectX::XMFLOAT4 HORIZON_COLOR = { 0.81f, 0.38f, 0.66f, 1.0f };
        static constexpr DirectX::XMFLOAT3 GROUND_COLOR = { 0.0f, 0.25f, 0.05f };

        // Light
        static constexpr DirectX::XMFLOAT3 LIGHT_DIR = { 0.5f, -1.0f, 0.5f };
        static constexpr DirectX::XMFLOAT4 LIGHT_DIFFUSE = { 255.0f * (3.0f / 255.0f), 250.0f * (3.0f / 255.0f), 245.0f * (3.0f / 255.0f), 1.0f };
        static constexpr DirectX::XMFLOAT4 LIGHT_AMBIENT = { 1.0f, 0.9f, 0.85f, 1.0f };
        static constexpr DirectX::XMFLOAT4 SUNSET_LIGHT_COLOR = { 255.0f * (5.0f / 255.0f), 120.0f * (5.0f / 255.0f),  30.0f * (5.0f / 255.0f), 1.0f };
        static constexpr DirectX::XMFLOAT4 NIGHT_LIGHT_COLOR = { 20.0f * (1.0f / 255.0f),  35.0f * (1.0f / 255.0f),  65.0f * (1.0f / 255.0f), 1.0f };

        // VolumeTextures
        static constexpr DirectX::XMFLOAT3 TEXTURE_SIZE = { 128.0f, 128.0f, 128.0f };
        static constexpr float             PERLIM_FREQ = 4.0f;
        static constexpr float             WORLEY_FREQ = 8.0f;
        static constexpr float             VOL_FREQ_G = 8.0f;
        static constexpr float             VOL_FREQ_B = 16.0f;
        static constexpr float             VOL_FREQ_A = 32.0f;
        static constexpr float             VOL_OCTAVES = 4;
        static constexpr float             VOL_REMAP_BIAS = 0.0f;

        // WeatherMap
        static constexpr DirectX::XMFLOAT2 WEATHER_MAP_RESOLUTION = { 1024.0f, 1024.0f };

        // Ground
        static constexpr DirectX::XMFLOAT3 DARK_SAND = { 0.6f, 0.45f, 0.3f };
        static constexpr DirectX::XMFLOAT3 LIGHT_SAND = { 0.85f, 0.7f, 0.5f };

        // Shadow
        static constexpr float             SHADOWMAP_WIDTH = 2048;
        static constexpr float             SHADOWMAP_HEIGHT = 2048;
        static constexpr float             BIAS = 0.005f;
        static constexpr float             SPREAD = 3.0f;

        // Cloud
		static constexpr float             CLOUD_RADIUS = 710000.0f;
        static constexpr float             CLOUD_BOTTOM = 5350.0f;
        static constexpr float             CLOUD_TOP = 6350.0f;

		static constexpr float             CLOUD_COVERAGE = 0.52f;
        static constexpr float             CLOUD_BASE_SCALE = 0.51f;
        static constexpr float             CLOUD_DETAIL_SCALE = 20.0f;

        static constexpr float             CLOUD_DENSITY = 0.03f;
        static constexpr float             CLOUD_BASE_EDGE_SOFTNESS = 0.25f;
        static constexpr float             CLOUD_BOTTOM_SOFTNESS = 0.25f;
        static constexpr float             CLOUD_DETAIL_STRENGTH = 0.225f;

        static constexpr float             CLOUD_FORWARD_SCATTERING_G = 0.2f;
        static constexpr float             CLOUD_BACKWARD_SCATTERING_G = 0.2f;
        static constexpr float             CLOUD_SCATTERING_LERP = 0.5f;
        static constexpr float             CLOUD_MIN_TRANSMITTANCE = 0.1f;

        static constexpr DirectX::XMFLOAT3 CLOUD_DAY_AMBIENT_COLOR_TOP = { 149.0f * (1.5f / 255.0f), 167.0f * (1.5f / 255.0f), 200.0f * (1.5f / 255.0f) };
        static constexpr DirectX::XMFLOAT3 CLOUD_DAY_AMBIENT_COLOR_BOTTOM = { 39.0f * (1.5f / 255.0f), 67.0f * (1.5f / 255.0f), 87.0f * (1.5f / 255.0f) };
        static constexpr DirectX::XMFLOAT3 CLOUD_SUNSET_AMBIENT_COLOR_TOP = { 180.0f * (1.5f / 255.0f), 100.0f * (1.5f / 255.0f), 60.0f * (1.5f / 255.0f) };
        static constexpr DirectX::XMFLOAT3 CLOUD_SUNSET_AMBIENT_COLOR_BOTTOM = { 80.0f * (1.5f / 255.0f), 50.0f * (1.5f / 255.0f), 40.0f * (1.5f / 255.0f) };
        static constexpr DirectX::XMFLOAT3 CLOUD_NIGHT_AMBIENT_COLOR_TOP = { 15.0f * (1.5f / 255.0f), 20.0f * (1.5f / 255.0f), 35.0f * (1.5f / 255.0f) };
        static constexpr DirectX::XMFLOAT3 CLOUD_NIGHT_AMBIENT_COLOR_BOTTOM = { 5.0f * (1.5f / 255.0f), 10.0f * (1.5f / 255.0f), 15.0f * (1.5f / 255.0f) };
        
        static constexpr DirectX::XMFLOAT2 WIND_DIRECTION = { 0.5f, -0.5f };
        static constexpr float             WIND_SPEED = 1.5f;
        static constexpr float             WIND_SCALE = 200.0f;

        static constexpr float             HENYEY_GREENSTEIN_SCALE = 1.0f;
        static constexpr float             POWDER_FACTOR = 0.4f;
        static constexpr float             LIGHTING_SCALE = 0.9f;
        static constexpr float             HORIZON_FADE_SCALE = 0.28f;

        // lensflare
        static constexpr DirectX::XMFLOAT3 LENSFLARE_RETOUCH_COLOR = { 1.4f, 1.2f, 1.0f };

        // GRASS
        static constexpr float             GRASS_BLADE_WIDTH = 5.0f;
		static constexpr float             GRASS_BLADE_HEIGHT = 7.0f;
        static constexpr float             WIND_STRENGTH = 1.0f;
        static constexpr float             DIST = 500.0f;
        static constexpr float             ALPHA_CUT = 0.3f;

        // 일반 상수
        static constexpr DirectX::XMFLOAT3 WATER_COLOR_SHALLOW = { 0.0f, 0.6f, 0.7f };
        static constexpr DirectX::XMFLOAT3 WATER_COLOR_DEEP = { 0.01f, 0.05f, 0.15f };

        static constexpr float             WATER_DISTORTION = 0.01f;
        static constexpr float             WATER_REFLECTIVITY = 0.6f;
        static constexpr float             WATER_DENSITY = 0.4f;
        static constexpr float             SUN_SHINIESS = 400.0f;

        static constexpr float             HEIGHT_SCALE = 1200.0f;
        static constexpr float             GRASS_DENSITY = 0.7f;
        static constexpr float             TREE_DENSITY = 0.005f;

        static constexpr float             TREE_BASE_OFFSET  = 5.0f;
        static constexpr float             TREE_SCALE_MIN    = 2.0f;
        static constexpr float             TREE_SCALE_MAX    = 8.0f;
        static constexpr float             TREE_JITTER_RANGE = 10.0f;
        static constexpr float             TREE_BLOCK_SIZE = 24.0f;
		static constexpr float             SEED_RANGE_CUT_OFF = 0.6f;
		static constexpr float             TREE_RANGE_CUT_OFF = 0.75f;
		static constexpr float             GRASS_RANGE_CUT_OFF = 0.4f;

        static constexpr float             FOG_BASE_HEIGHT = 0.15f;
        static constexpr float             FOG_HEIGHT_FALLOFF = 0.01f;
        static constexpr float             FOG_DENSITY = 0.15f;
        static constexpr float             FOG_MAX_DISTANCE = 3000.0f;
        static constexpr float             FOG_NOISE_SCALE = 0.0008f;
        static constexpr float             FOG_NOISE_STRENGTH = 0.6f;
        static constexpr float             FOG_WIND_SPEED = 0.5f;
        static constexpr float             FOG_PHASE_G = 0.3f;
        static constexpr DirectX::XMFLOAT3 FOG_COLOR = { 0.7f, 0.75f, 0.8f };
        static constexpr float             FOG_AMBIENT_STRENGTH = 1.0f;
        static constexpr float             FOG_SLOPE_MIN = 0.3f;
        static constexpr float             FOG_SLOPE_MAX = 1.0f;
        static constexpr DirectX::XMFLOAT2 FOG_WIND_DIRECTION = { 1.0f, 0.0f };
        static constexpr int               FOG_MARCH_STEPS = 24;

    } // BuffersConstants
}