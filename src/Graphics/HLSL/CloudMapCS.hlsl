// CloudMapCS.hlsl
// 참고 : https://github.com/chihirobelmo/volumetric-cloud-for-directx11/blob/main/VolumetricCloud/shaders/CloudMapGenerate.hlsl
#include "FBM.hlsli"
#include "Remap.hlsli"

RWTexture2D<float4> outCloudMap : register(u0);

cbuffer CloudMapBuffer : register(b0)
{
    // [Row1]
    float2 cResolution;
    float  cTime;
    float  cCoverage; // 구름 분포도 (0.0: 맑음 ~ 1.0: 흐림)
    // [Row2]
    float  cPerlinFbmFreq; // 8
    float  cPerlinOctaves; // 4
    float  cPerlinBias; // 0.25
    float  cWorleyFbmFreq; // 8
    // [Row3]
    float2 cWindDirection; // 바람이 부는 2D 방향 (예: 1.0, 0.0 이면 X축 이동)
    float  cWindSpeed; // 바람의 세기
    float  cPadding;
}; // CloudMapBuffer

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // 스레드가 텍스처 범위를 벗어나지 않도록 방어
    if (DTid.x >= (uint) cResolution.x || DTid.y >= (uint) cResolution.y)
        return;

    float2 uv = float2(DTid.xy) / cResolution;

    // 바람 벡터 계산 (방향 * 속도 * 시간)
    float2 windOffset = cWindDirection * cWindSpeed * cTime;

    // UV 스크롤 적용
    // UV에서 오프셋을 빼주면 구름이 윈드 방향(cWindDirection)으로 흘러감
    float2 movingUV = uv - windOffset;

    // 3D 노이즈 함수용 UVW 좌표 생성
    // 2D 텍스처를 만들지만 노이즈 함수가 3D를 요구하므로 Z는 0.0으로 고정합
    float3 uvw_wind = float3(movingUV * 2.0 - 1.0, 0.0);

    // 텍스처 데이터 생성
    // R 채널: Cloud Coverage (구름의 분포와 베이스 덩어리)
    float p = perlin_fbm(uvw_wind, cPerlinFbmFreq, cPerlinOctaves) * cPerlinBias;
    float w_base = worley_fbm(uvw_wind, cWorleyFbmFreq, true);
    float pw = remap_new(p, w_base - 1.0, 1.0, 0.0, 1.0);

    // G 채널: Cloud Type (구름의 종류, 거칠기 등)
    float w = worley_fbm(uvw_wind, 8.0, true);

    // 전체 구름 양(cCoverage)에 따른 Remap 조절
    float r = remap_clamp(pw * 0.5 + 0.5, 1.0 - cCoverage, 1.0, 0.0, 1.0);
    float g = remap_clamp(w * 0.5 + 0.5, 1.0 - cCoverage, 1.0, 0.0, 1.0);

    // (R: Coverage, G: Type)
    outCloudMap[DTid.xy] = float4(r, g, 0.0, 1.0);
} // main