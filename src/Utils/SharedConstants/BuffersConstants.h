#pragma once
#include <iostream>
#include <directxmath.h>
#include "Utils/SharedConstants/ScreenConstants.h"

namespace SharedConstants {

    namespace BuffersConstants {

        // Atmoshpere
        static constexpr float PLANET_RADIUS = 6371.0f;
        static constexpr float ATMOSPHERE_RADIUS = 6471.0;
        static constexpr float ATMOSPHERE_HEIGHT = 100.0f;
        static constexpr float RAYLEIGH_HEIGHT = 8.0f;
        static constexpr float MIE_HEIGHT = 1.2;
        static constexpr float ABSORPTION_HEIGHT = 30.0f;
        static constexpr float ABSORPTION_FALLOFF = 4.0f;

        static constexpr DirectX::XMFLOAT3 RAYLEIGH_SCATTERING_COEFFICIENT = { 5.8e-3f, 13.5e-3f, 33.1e-3f };
        static constexpr float             MIE_BETA = 21e-3f;
        static constexpr DirectX::XMFLOAT3 MIE_SCATTERING_COEFFICIENT = { 21e-3f, 21e-3f, 21e-3f };

        static constexpr float             AMBIENT_BETA = 0.0f;
        static constexpr float             G = 0.9f;
        static constexpr float             ATMOSPHERE_INTENWSITY = 40.0f;

        static constexpr DirectX::XMFLOAT4 ZENITH_COLOR = { 0.0f, 0.2f, 0.6f, 1.0f };
        static constexpr DirectX::XMFLOAT4 HORIZON_COLOR = { 0.81f, 0.38f, 0.66f, 1.0f };
        static constexpr DirectX::XMFLOAT3 GROUND_COLOR = { 0.0f, 0.25f, 0.05f };


        // Light
        static constexpr DirectX::XMFLOAT3 LIGHT_DIR = { 0.5f, -1.0f, 0.5f };
        static constexpr DirectX::XMFLOAT4 LIGHT_DIFFUSE = { 1.0f, 1.0f, 1.0f, 1.0f };
        static constexpr DirectX::XMFLOAT4 LIGHT_AMBIENT = { 0.15f, 0.15f, 0.15f, 1.0f };


        // VolumeTextures
        static constexpr DirectX::XMFLOAT3 TEXTURE_SIZE = { 128.0f, 128.0f, 128.0f };
        static constexpr float             PERLIM_FREQ = 4.0f;
        static constexpr float             WORLEY_FREQ = 8.0f;
        static constexpr float             VOL_FREQ_G = 8.0f;
        static constexpr float             VOL_FREQ_B = 16.0f;
        static constexpr float             VOL_FREQ_A = 32.0f;
        static constexpr float             VOL_OCTAVES = 4;
        static constexpr float             VOL_REMAP_BIAS = 0.0f;


        // CloudMap
        static constexpr DirectX::XMFLOAT2 RESOLUTION = { SharedConstants::ScreenConstants::WIDTH, SharedConstants::ScreenConstants::HEIGHT };
        static constexpr float             COVERAGE = 0.4f;
        static constexpr float             PERLIN_FBM_FREQ = 8.0f; // 8
        static constexpr float             PERLIN_OCTAVES = 4.0f; // 4
        static constexpr float             PERLIN_BIAS = 0.25f; // 0.25
        static constexpr float             WORLEY_FBM_FREQ = 8.0f; // 8
        static constexpr DirectX::XMFLOAT2 WIND_DIRECTION = { 1.0f, 0.0f }; // 바람이 부는 2D 방향 (예: 1.0, 0.0 이면 X축 이동)
        static constexpr float             WIND_SPEED = 0.4f; // 바람의 세기

        // Ground
        static constexpr DirectX::XMFLOAT3 DARK_SAND = { 0.6f, 0.45f, 0.3f };
        static constexpr DirectX::XMFLOAT3 LIGHT_SAND = { 0.85f, 0.7f, 0.5f };

        // Shadow
        static constexpr float             SHADOWMAP_WIDTH = 2048;
        static constexpr float             SHADOWMAP_HEIGHT = 2048;
        static constexpr float             BIAS = 0.001f;
        static constexpr float             SPREAD = 2.0f;

        // Cloud
        static constexpr float             CLOUD_MIN_HEIGHT = 1.5f;
        static constexpr float             CLOUD_MAX_HEIGHT = 2.5f;

    } // BuffersConstants
}