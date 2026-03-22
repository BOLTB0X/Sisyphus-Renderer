// NoiseGeneratorCS.hlsl
// chihirobelmo cloudmap부분 참고: https://github.com/chihirobelmo/volumetric-cloud-for-directx11/blob/main/VolumetricCloud/shaders/CloudMapGenerate.hlsl
// maximeheckel https://blog.maximeheckel.com/posts/real-time-cloudscapes-with-volumetric-raymarching/
#include "FBM.hlsl"
#include "Remap.hlsl"

RWTexture3D<float4> outVolume : register(u0);

cbuffer NoiseBuffer : register(b0)
{
    // [Row 1]
    float3 nTextureSize; // 텍스처 해상도
    float  nPerlinFreq; // 기본 덩어리 Perlin 주파수
    // [Row 2]
    float  nWorleyFreq;  // 기본 덩어리 Worley 주파수
    float  nDetailFreqG; // G 채널 디테일 주파수
    float  nDetailFreqB; // B 채널 디테일 주파수
    float  nDetailFreqA; // A 채널 디테일 주파수
    // [Row 3]
    int    nOctaves;   // FBM 반복 횟수
    float  nRemapBias; // 구름 밀도 조절용 Bias
    float2 nPadding;
}; // NoiseBuffer

[numthreads(8, 8, 8)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    // 스레드 ID가 텍스처 범위를 벗어나지 않도록 방어
    if (any(dispatchThreadId >= (uint3) nTextureSize))
        return;

    float3 uvw = float3(dispatchThreadId) / nTextureSize;

    // R 채널
    float pfbm = perlin_fbm(uvw, nPerlinFreq, nOctaves);
    float wBase = worley_periodic(uvw, nWorleyFreq);
    float cloudBase = remap_clamp(pfbm, wBase - 1.0, 1.0, 0.0, 1.0);
    cloudBase = saturate(cloudBase + nRemapBias);

    // G, B, A 채널
    float detailG = worley_periodic(uvw, nDetailFreqG);
    float detailB = worley_periodic(uvw, nDetailFreqB);
    float detailA = worley_periodic(uvw, nDetailFreqA);
    
    // R: Base Shape
    // GBA: Erosion Details
    //outVolume[dispatchThreadId] = float4(1, 1, 1, 1);
    outVolume[dispatchThreadId] = saturate(float4(cloudBase, detailG, detailB, detailA));
} // main