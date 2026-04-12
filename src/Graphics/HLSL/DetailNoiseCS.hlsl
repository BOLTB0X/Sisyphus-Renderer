// DetailNoiseCS.hlsl
// https://github.com/fede-vaccaro/TerrainEngine-OpenGL/blob/master/shaders/worley.comp
// https://www.shadertoy.com/view/ttcSD8
#include "Noise.hlsli"
#include "Remap.hlsli"

RWTexture3D<float4> outVolume : register(u0);

cbuffer NoiseBuffer : register(b3)
{
    float3 nTextureSize; //32 x 32 x 32
    float  npadding;
}; // NoiseBuffer

float4 Stackable3DNoise(int3 pixel)
{
    float3 coord = float3(float(pixel.x) / 32.0f, float(pixel.y) / 32.0f, float(pixel.z) / 32.0f);

	// 3 octaves
    float cellCount = 2.0f;
    float worleyNoise0 = (1.0f - worley_noise_3D(coord, cellCount * 1.0f));
    float worleyNoise1 = (1.0f - worley_noise_3D(coord, cellCount * 2.0f));
    float worleyNoise2 = (1.0f - worley_noise_3D(coord, cellCount * 4.0f));
    float worleyNoise3 = (1.0f - worley_noise_3D(coord, cellCount * 8.0f));
    float worleyFBM0 = worleyNoise0 * 0.625f + worleyNoise1 * 0.25f + worleyNoise2 * 0.125f;
    float worleyFBM1 = worleyNoise1 * 0.625f + worleyNoise2 * 0.25f + worleyNoise3 * 0.125f;
    float worleyFBM2 = worleyNoise2 * 0.75f + worleyNoise3 * 0.25f;
	// cellCount=4 -> worleyNoise4는 샘플링 주파수(텍셀 주파수와 동일)로 인해 발생하는 노이즈일 뿐
    // 따라서 FBM에서는 두 개의 주파수만 고려

    return float4(worleyFBM0, worleyFBM1, worleyFBM2, 1.0);
} // Stackable3DNoise

[numthreads(4, 4, 4)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (any(DTid >= (uint3) nTextureSize))
    {
        return;
    }

    float4 finalNoise = Stackable3DNoise((int3) DTid);
    outVolume[DTid] = finalNoise;
} // main