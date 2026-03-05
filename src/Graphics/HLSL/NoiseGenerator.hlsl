cbuffer NoiseBuffer : register(b0)
{
    float3 textureSize; // 텍스처 해상도
    float perlinFreq; // 기본 덩어리 Perlin 주파수

    float worleyFreq; // 기본 덩어리 Worley 주파수
    float detailFreqG; // G 채널 디테일 주파수
    float detailFreqB; // B 채널 디테일 주파수
    float detailFreqA; // A 채널 디테일 주파수

    int octaves; // FBM 반복 횟수
    float remapBias; // 구름 밀도 조절용 Bias
    float2 padding; // 16바이트 정렬을 위한 패딩
}; // NoiseBuffer

RWTexture3D<float4> outVolume : register(u0);

float remap(float value, float originMin, float originMax, float targetMin, float targetMax)
{
    return targetMin + (value - originMin) * (targetMax - targetMin) / (originMax - originMin);
} // remap

float3 hash33(float3 p)
{
    uint3 q = uint3(int3(p)) * uint3(1597334673U, 3812015801U, 2798796415U);
    q = (q.x ^ q.y ^ q.z) * uint3(1597334673U, 3812015801U, 2798796415U);
    return -1.0 + 2.0 * float3(q) / 4294967295.0;
} // hash33

// 타일링 가능한 Worley 노이즈
float worleyPeriodic(float3 p, float freq)
{
    float3 samplePos = p * freq;
    float3 cell = floor(samplePos);
    float3 local = frac(samplePos);
    float minDist = 1.0;

    for (int z = -1; z <= 1; z++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int x = -1; x <= 1; x++)
            {
                float3 neighbor = float3(x, y, z);
                // 주기적 경계 조건 (Tiling)
                float3 wrappedCell = fmod(cell + neighbor + freq, freq);
                float3 featurePoint = hash33(wrappedCell) * 0.5 + 0.5;
                
                float3 diff = neighbor + featurePoint - local;
                minDist = min(minDist, dot(diff, diff));
            }
        }
    }
    return 1.0 - sqrt(minDist);
} // worleyPeriodic

// 타일링 가능한 Perlin-like FBM
float perlinFbm(float3 p, float freq, int octaveCount)
{
    float g = 0.5;
    float amp = 1.0;
    float noise = 0.0;
    for (int i = 0; i < octaveCount; ++i)
    {
        noise += amp * worleyPeriodic(p, freq);
        freq *= 2.0;
        amp *= g;
    }
    return noise;
} // perlinFbm

[numthreads(8, 8, 8)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    // 스레드 ID가 텍스처 범위를 벗어나지 않도록 방어
    if (any(dispatchThreadId >= (uint3) textureSize))
        return;

    float3 uvw = float3(dispatchThreadId) / textureSize;

    // R 채널: Perlin-Worley 합성 -> 거대 골격
    float pw = perlinFbm(uvw, perlinFreq, octaves) * 0.3;
    float w = worleyPeriodic(uvw, worleyFreq);
    
    // remapBias를 통해 기본 구름의 "살집"을 조절
    float cloudBase = remap(pw, 1.0 - w, 1.0, 0.0, 1.0) + remapBias;

    // G, B, A 채널: 서로 다른 주파수의 Worley FBM
    // 각 채널마다 주파수를 다르게 주어 다양한 크기의 침식(Erosion) 효과를 만듬
    float detailG = worleyPeriodic(uvw, detailFreqG) * 0.625 + worleyPeriodic(uvw, detailFreqG * 2.0) * 0.25;
    float detailB = worleyPeriodic(uvw, detailFreqB) * 0.625 + worleyPeriodic(uvw, detailFreqB * 2.0) * 0.25;
    float detailA = worleyPeriodic(uvw, detailFreqA) * 0.625 + worleyPeriodic(uvw, detailFreqA * 2.0) * 0.25;

    outVolume[dispatchThreadId] = saturate(float4(cloudBase, detailG, detailB, detailA));
} // main