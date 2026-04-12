// AtmosphereCS.hlsl
// https://www.shadertoy.com/view/wlBXWK
#include "Common.hlsli"
#include "Atmosphere.hlsli"

cbuffer AtmosphereBuffer : register(b3)
{
    // [Row 1] 단순 그라데이션
    float4 aZenithColor;
    // [Row 2] 단순 그라데이션
    float4 aHorizonColor;
    // [Row 3] 행성 물리 데이터
    float3 aPlanetCenter;
    float  aPlanetRadius;
    // [Row 4] 대기권 물리 데이터
    float  aAtmoRadius;
    float3 aPadding;
    // [Row 5] 산란 계수 (Rayleigh)
    float3 aRayleighBeta;
    float  aMieBeta;
    // [Row 6] 흡수 및 주변광
    float3 aAbsorptionBeta;
    float  aAmbientBeta;
    // [Row 7] 고도 상수 (Density Falloff)
    float  aRayleighHeight;
    float  aMieHeight;
    float  aAbsorptionHeight;
    float  aAbsorptionFalloff;
    // [Row 8] Mie 위상 함수 및 샘플링 설정
    float  aG;
    int    aPrimarySteps;
    int    aLightSteps;
    float  aIntensity;
    // [Row 9] 지표면 색상
    float3 aGroundColor;
    float  aPadding2;
    // [Row 10] 지표면 레이마칭 설정
    int    aGroundPrimarySteps;
    int    agroundLightSteps;
    float2 aPadding3;
}; // AtmosphereBuffer

cbuffer ResolutionBuffer : register(b4)
{
    float2 rResolution;
    float2 rPadding;
} // ResolutionBuffer

SamplerState        LinearSampler : register(s0);
RWTexture2D<float4> OutTexture : register(u0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint width, height;
    OutTexture.GetDimensions(width, height);

    if (DTid.x >= width || DTid.y >= height )
    {
        return;
    }

    float2 uv = (float2(DTid.xy) + 0.5f) / rResolution;
    float theta = (uv.x - 0.5f) * 2.0f * PI; // -PI ~ PI
    float phi = (0.5f - uv.y) * PI; // -PI/2 ~ PI/2
    
    float3 rd = float3(cos(phi) * sin(theta), sin(phi), cos(phi) * cos(theta));
    float3 ro = cCameraPosition / KM;
    float max_dist = MAX_DIST;

    float2 planet_intersect = ray_sphere_intersect(ro - aPlanetCenter, rd, aPlanetRadius - 0.1f);
    float groundDist = (planet_intersect.x > 0) ? planet_intersect.x : max_dist;
    float3 scene_color = dot(rd, cLightDirection) > 0.9998 ? 3.0 : 0.0;

    if (planet_intersect.y > 0.0f)
    {
        max_dist = max(planet_intersect.x, 0.0);
        scene_color = calculate_ground_scattering(ro, rd, planet_intersect,
            aGroundColor, 3.0 * aAtmoRadius, ORIGIN, cLightDirection, float3(aIntensity, aIntensity, aIntensity),
            aPlanetCenter, aPlanetRadius, aAtmoRadius,
            aRayleighBeta, aMieBeta, aAbsorptionBeta, float3(aAmbientBeta, aAmbientBeta, aAmbientBeta),
            aG, aRayleighHeight, aMieHeight, aAbsorptionHeight, aAbsorptionFalloff,
            aGroundPrimarySteps, agroundLightSteps);

    }

    float3 col = calculate_atmosphere_scattering(
        ro, rd, max_dist,
        scene_color,
        -cLightDirection,
        float3(aIntensity, aIntensity, aIntensity),
        aPlanetCenter,
        aPlanetRadius,
        aAtmoRadius,
        aRayleighBeta,
        aMieBeta,
        aAbsorptionBeta,
        float3(aAmbientBeta, aAmbientBeta, aAmbientBeta),
        aG,
        aRayleighHeight,
        aMieHeight,
        aAbsorptionHeight,
        aAbsorptionFalloff,
        aPrimarySteps,
        aLightSteps
    );

    OutTexture[DTid.xy] = float4(col, 1.0f);
} // main