// WorleyNoiseCS.hlsl
// https://github.com/fede-vaccaro/TerrainEngine-OpenGL/blob/master/shaders/perlinworley.comp
// https://github.com/chihirobelmo/volumetric-cloud-for-directx11/blob/main/VolumetricCloud/shaders/CloudMapGenerate.hlsl
// https://github.com/sebh/TileableVolumeNoise
// https://www.shadertoy.com/view/ttcSD8
#include "Noise.hlsli"

RWTexture3D<float4> OutCloud3DLUT : register(u0);

[numthreads(8, 8, 8)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float3 dims;
    OutCloud3DLUT.GetDimensions(dims.x, dims.y, dims.z);
    
    if (DTid.x >= dims.x || DTid.y >= dims.y || DTid.z >= dims.z)
        return;

    float3 coord = float3(DTid) / dims;

    // 3D 보로노이 노이즈 레이어링
    float r = tilable_voronoi(coord, 16.0f, 3.0f);
    float g = tilable_voronoi(coord, 4.0f, 8.0f);
    float b = tilable_voronoi(coord, 4.0f, 16.0f);

    // 노이즈 반전 및 합성 (Erosion Tool 만들기)
    // r, g, b를 섞어 구름의 "살"을 깎아낼 최종 값을 계산
    float c = max(0.0f, 1.0f - (r + g * 0.5f + b * 0.25f) / 1.75f);

    // 3D 텍스처(UAV)에 기록
    OutCloud3DLUT[DTid] = float4(c, c, c, c);
} // main