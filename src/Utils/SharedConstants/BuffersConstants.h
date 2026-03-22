#pragma once
#include <iostream>
#include <directxmath.h>

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


        // VolumeTextures
        static constexpr DirectX::XMFLOAT3 TEXTURE_SIZE = { 128.0f, 128.0f, 128.0f };
        static constexpr float             PERLIM_FREQ = 4.0f;
        static constexpr float             WORLEY_FREQ = 8.0f;
        static constexpr float             VOL_FREQ_G = 8.0f;
        static constexpr float             VOL_FREQ_B = 16.0f;
        static constexpr float             VOL_FREQ_A = 32.0f;
        static constexpr float             VOL_OCTAVES = 4;
        static constexpr float             VOL_REMAP_BIAS = 0.0f;

        static constexpr float             CLOUD_MIN_HEIGHT = 2000.0f;
        static constexpr float             CLOUD_MAX_HEIGHT = 4000.0f;

    } // BuffersConstants
}