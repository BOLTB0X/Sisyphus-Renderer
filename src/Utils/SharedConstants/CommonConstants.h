#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <wrl/client.h>

namespace SharedConstants {
	namespace CommonConstants {
		static float QUAD_MAX_LENG = 800.0f;
		static float HEIGHT_SCALE = 250.0f;
		static float QUAD_SCALE = 10.0f;

        static UINT  MAX_TRIANGLES_PER_NODE = 10000;
        static int   GRASS_SEED_STEP        = 3;
        static float GRASS_PER_TRIANGLE     = 6;

		static float WATER_HEIGHT = 97.0f;
		static float WATER_SCALE = 80000.0f;
		static int   GRID_DENISTY = 3;
	} // CommonConstants
}