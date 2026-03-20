// NoiseGeneratorCS.hlsl
#include "FBM.hlsl"
#include "Remap.hlsl"

cbuffer NoiseBuffer : register(b0)
{
    // [Row 1]
    float3 nTextureSize; // 텍스처 해상도
    float  nPerlinFreq; // 기본 덩어리 Perlin 주파수
    // [Row 2]
    float  nWorleyFreq; // 기본 덩어리 Worley 주파수
    float  nDetailFreqG; // G 채널 디테일 주파수
    float  nDetailFreqB; // B 채널 디테일 주파수
    float  nDetailFreqA; // A 채널 디테일 주파수
    // [Row 3]
    int    nOctaves; // FBM 반복 횟수
    float  nRemapBias; // 구름 밀도 조절용 Bias
    float2 nPadding; // 16바이트 정렬을 위한 패딩
}; // NoiseBuffer

RWTexture3D<float4> outVolume : register(u0);

[numthreads(8, 8, 8)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    // 스레드 ID가 텍스처 범위를 벗어나지 않도록 방어
    if (any(dispatchThreadId >= (uint3) nTextureSize))
        return;

    float3 uvw = float3(dispatchThreadId) / nTextureSize;

    // R 채널: Perlin-Worley (구름의 거대 구조)
    // Valentin Galea의 fbm_clouds(abs noise) 느낌을 위해 Perlin FBM을 기본으로 사용
    float pfbm = perlinFbm(uvw, nPerlinFreq, nOctaves);
    float wBase = WorleyPeriodic(uvw, nWorleyFreq);
    
    // Perlin 노이즈를 Worley 노이즈로 리맵하여 뭉게구름의 외곽선(Billowy) 형성
    float cloudBase = remap(pfbm, wBase - 1.0, 1.0, 0.0, 1.0);
    cloudBase = saturate(cloudBase + nRemapBias);

    // G, B, A 채널: 다양한 주파수의 Worley FBM (디테일 및 침식용)
    // 점진적으로 주파수를 높여서 PS에서 "깎아낼" 디테일을 제공
    float detailG = WorleyPeriodic(uvw, nDetailFreqG);
    float detailB = WorleyPeriodic(uvw, nDetailFreqB);
    float detailA = WorleyPeriodic(uvw, nDetailFreqA);
    
    // R: Base Shape
    // GBA: Erosion Details
    outVolume[dispatchThreadId] = saturate(float4(cloudBase, detailG, detailB, detailA));
} // main