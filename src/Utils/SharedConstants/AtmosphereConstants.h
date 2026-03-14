#pragma once
#include <iostream>
#include <directxmath.h>

namespace SharedConstants {

    namespace AtmosphereConstants {
        static constexpr float PLANET_RADIUS = 6371.0f;
		static constexpr float ATMOSPHERE_RADIUS = 6471.0;
        static constexpr float ATMOSPHERE_HEIGHT = 100.0f;
        static constexpr float RAYLEIGH_HEIGHT = 8.0f;
		static constexpr float MIE_HEIGHT = 1.2;
        static constexpr float ABSORPTION_HEIGHT = 30.0f;
		static constexpr float ABSORPTION_FALLOFF = 4.0f;

        static constexpr DirectX::XMFLOAT3 RAYLEIGH_SCATTERING_COEFFICIENT = { 5.8e-3f, 13.5e-3f, 33.1e-3f }; 
        static constexpr float MIE_BETA = 21e-3f; 
        static constexpr DirectX::XMFLOAT3 MIE_SCATTERING_COEFFICIENT = { 21e-3f, 21e-3f, 21e-3f }; 

        static constexpr float AMBIENT_BETA = 0.0f;
    } // AtmosphereConstants
}