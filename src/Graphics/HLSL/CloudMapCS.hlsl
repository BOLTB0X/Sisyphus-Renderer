// CloudMapCS.hlsl
// https://www.shadertoy.com/view/MdGfzh
#include "Noise.hlsli"
// define
#define MIX_FBM     0.9f
#define MIX_VORONOI 0.7f

RWTexture2D<float4> OutCloud2DLUT : register(u0);

const static float mixFbm = MIX_FBM;
const static float mixVor = MIX_VORONOI;


[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float width, height;
    OutCloud2DLUT.GetDimensions(width, height);
    
    if (DTid.x >= width || DTid.y >= height)
        return;

    // UV 및 좌표 설정
    float2 vUV = float2(DTid.xy) / float2(width, height);
    float3 coord = frac(float3(vUV + float2(0.2f, 0.62f), 0.5f));

    float4 col = float4(1.0f, 1.0f, 1.0f, 1.0f);

    // R 채널: 기본 밀도
    col.r = lerp(1.0f, tilable_fbm(coord, 7, 4.0f), mixFbm) * lerp(1.0f, tilable_voronoi(coord, 8, 9.0f), mixVor);

    // G 채널: 디테일 (Voronoi 옥타브 합)
    col.g = 0.625f * tilable_voronoi(coord, 3, 15.0f) +
            0.250f * tilable_voronoi(coord, 3, 19.0f) +
            0.125f * tilable_voronoi(coord, 3, 23.0f) - 1.0f;

    // B 채널: 구름 타입/높이
    col.b = 1.0f - tilable_voronoi(coord + 0.5f, 6, 9.0f);

    OutCloud2DLUT[DTid.xy] = col;
} // main