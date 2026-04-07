// BaseNoiseCS.hlsl
// chihirobelmo cloudmap부분 참고: https://github.com/chihirobelmo/volumetric-cloud-for-directx11/blob/main/VolumetricCloud/shaders/CloudMapGenerate.hlsl
// maximeheckel https://blog.maximeheckel.com/posts/real-time-cloudscapes-with-volumetric-raymarching/
// Shader Toy: https://www.shadertoy.com/view/ttcSD8
#include "FBM.hlsli"
#include "Remap.hlsli"

RWTexture3D<float4> outVolume : register(u0);

cbuffer NoiseBuffer : register(b3)
{
    float3 cTextureSize; // 128x128x128
    float cpadding;
}; // NoiseBuffer

[numthreads(8, 8, 8)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (any(DTid >= (uint3) cTextureSize))
        return;
    
    float3 uvw = (float3(DTid) + 0.5f) / cTextureSize;
    
    float worley = worley_fbm(uvw, 8, true);
    float perlin = perlin_fbm(uvw, 8, 4);
    float perlinWorley = remap_new(perlin * 0.25 + 0.5, 1.0 - worley, 1.0, 0.0, 1.0);

    float g = worley_fbm(uvw, 6, true);
    float b = worley_fbm(uvw, 12, true);
    float a = worley_fbm(uvw, 24, true);
    
    outVolume[DTid] = float4(perlinWorley, g, b, a);
} // main