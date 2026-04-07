// DetailNoiseCS.hlsl
// chihirobelmo cloudmap부분 참고: https://github.com/chihirobelmo/volumetric-cloud-for-directx11/blob/main/VolumetricCloud/shaders/CloudMapGenerate.hlsl
// maximeheckel https://blog.maximeheckel.com/posts/real-time-cloudscapes-with-volumetric-raymarching/
// Shader Toy: https://www.shadertoy.com/view/ttcSD8
#include "FBM.hlsli"
#include "Remap.hlsli"

RWTexture3D<float4> outVolume : register(u0);

cbuffer NoiseBuffer : register(b3)
{
    float3 cTextureSize; //32 x 32 x 32
    float  cpadding;
}; // NoiseBuffer

[numthreads(4, 4, 4)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (any(DTid >= (uint3) cTextureSize))
        return;

    float3 uvw = (float3(DTid) + 0.5f) / cTextureSize;
    
    float r = worley_fbm(uvw, 3, true);
    float g = worley_fbm(uvw, 6, true);
    float b = worley_fbm(uvw, 9, true);
    float a = blue_noise(uvw + uvw.z, 32);

    outVolume[DTid] = float4(r, g, b, a);
} // main