// BaseNoiseCS.hlsl
// https://github.com/fede-vaccaro/TerrainEngine-OpenGL/blob/master/shaders/perlinworley.comp
// https://github.com/chihirobelmo/volumetric-cloud-for-directx11/blob/main/VolumetricCloud/shaders/CloudMapGenerate.hlsl
// https://github.com/sebh/TileableVolumeNoise
// https://www.shadertoy.com/view/ttcSD8
#include "Noise.hlsli"
#include "Remap.hlsli"

cbuffer NoiseBuffer : register(b3)
{
    float3 nTextureSize; // 128x128x128
    float  npadding;
}; // NoiseBuffer

RWTexture3D<float4> outVolume : register(u0);

float4 Stackable3DNoise(int3 pixel)
{
    float3 coord = float3(float(pixel.x) / 128.0, float(pixel.y) / 128.0, float(pixel.z) / 128.0);

	// Perlin FBM noise
    int octafloatount = 3;
    float frequency = 8.0;
    float perlinNoise = perlin_noise_3D(coord, frequency, octafloatount);

    float PerlinWorleyNoise = 0.0f;
	{
        float cellCount = 4.0;
        float worleyNoise0 = (1.0 - worley_noise_3D(coord, cellCount * frequenceMul[0]));
        float worleyNoise1 = (1.0 - worley_noise_3D(coord, cellCount * frequenceMul[1]));
        float worleyNoise2 = (1.0 - worley_noise_3D(coord, cellCount * frequenceMul[2]));
        float worleyNoise3 = (1.0 - worley_noise_3D(coord, cellCount * frequenceMul[3]));
        float worleyNoise4 = (1.0 - worley_noise_3D(coord, cellCount * frequenceMul[4]));
        float worleyNoise5 = (1.0 - worley_noise_3D(coord, cellCount * frequenceMul[5])); //텍셀 빈도가 절반이면 더 이상 진행해서는 안 된다함cellCount = 32인 경우). texture size = 64)

		// GPU Pro 7의 101페이지에 설명된 PerlinWorley 노이즈
        float worleyFBM = worleyNoise0 * 0.625f + worleyNoise1 * 0.25f + worleyNoise2 * 0.125f;

        PerlinWorleyNoise = remap_new(perlinNoise, 0.0, 1.0, worleyFBM, 1.0);
    }

    float cellCount = 4.0;
    float worleyNoise0 = (1.0 - worley_noise_3D(coord, cellCount * 1.0));
    float worleyNoise1 = (1.0 - worley_noise_3D(coord, cellCount * 2.0));
    float worleyNoise2 = (1.0 - worley_noise_3D(coord, cellCount * 4.0));
    float worleyNoise3 = (1.0 - worley_noise_3D(coord, cellCount * 8.0));
    float worleyNoise4 = (1.0 - worley_noise_3D(coord, cellCount * 16.0));
	//float worleyNoise5 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 32));	
	//cellCount=2 -> 텍셀 빈도의 절반이므로 더 이상 진행해서는 안 됌(cellCount=32, 텍스처 크기=64인 경우)

	//  Worley FBM 노이즈의 세 가지 주파수
    float worleyFBM0 = worleyNoise1 * 0.625f + worleyNoise2 * 0.25f + worleyNoise3 * 0.125f;
    float worleyFBM1 = worleyNoise2 * 0.625f + worleyNoise3 * 0.25f + worleyNoise4 * 0.125f;
	//float worleyFBM2 = worleyNoise3*0.625f + worleyNoise4*0.25f + worleyNoise5*0.125f;
    float worleyFBM2 = worleyNoise3 * 0.75f + worleyNoise4 * 0.25f;
	// cellCount=4 -> worleyNoise5는 샘플링 주파수(텍셀 주파수와 동일)로 인한 노이즈일 뿐
    // 따라서 FBM에서는 두 가지 주파수만 고려해야 한다함

    return float4(PerlinWorleyNoise * PerlinWorleyNoise, worleyFBM0, worleyFBM1, worleyFBM2);
} // Stackable3DNoise

[numthreads(8, 8, 8)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (any(DTid >= (uint3) nTextureSize))
    {
        return;
    }
    
    float4 finalNoise = Stackable3DNoise((int3) DTid);
    outVolume[DTid] = finalNoise;
} // main