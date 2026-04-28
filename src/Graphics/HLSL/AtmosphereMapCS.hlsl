// AtmosphereMapCS.hlsl
// https://www.shadertoy.com/view/wlBXWK
#include "Common.hlsli"
#include "Atmosphere.hlsli"

cbuffer AtmosphereBuffer : register(b2)
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
    int    aGroundLightSteps;
    float2 aPadding3;
}; // AtmosphereBuffer

cbuffer ResolutionBuffer : register(b3)
{
    float2 rResolution;
    float2 rPadding;
} // ResolutionBuffer

SamplerState        LinearSampler : register(s0);
RWTexture2D<float4> OutTexture : register(u0);

#define ZENITH_COLOR       aZenithColor.rgb
#define HORIZON_COLOR      aHorizonColor.rgb
#define PLANET_CENTER      aPlanetCenter
#define PLANET_RADIUS      aPlanetRadius
#define ATMO_RADIUS        aAtmoRadius
#define RAYLEIGH_BETA      aRayleighBeta
#define MIE_BETA           aMieBeta
#define ABSORPTION_BETA    aAbsorptionBeta
#define AMBIENT_BETA       aAmbientBeta
#define RAYLEIGH_HEIGHT    aRayleighHeight
#define MIE_HEIGHT         aMieHeight
#define ABSORPTION_HEIGHT  aAbsorptionHeight
#define ABSORPTION_FALLOFF aAbsorptionFalloff
#define G                  aG

#define PRIMARY_STEPS        aPrimarySteps
#define LIGHT_STEPS          aLightSteps
#define INTENSITY            aIntensity
#define GROUND_COLOR         aGroundColor
#define GROUND_PRIMARY_STEPS aGroundPrimarySteps
#define GROUND_LIGHT_STEPS   aGroundLightSteps
#define RESOLUTION           rResolution          

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint width, height;
    OutTexture.GetDimensions(width, height);

    if (DTid.x >= width || DTid.y >= height )
    {
        return;
    }

    float2 uv = (float2(DTid.xy) + 0.5f) / RESOLUTION;
    float theta = (uv.x - 0.5f) * 2.0f * PI; // -PI ~ PI
    float phi = (0.5f - uv.y) * PI; // -PI/2 ~ PI/2
    
    float3 rd = float3(cos(phi) * sin(theta), sin(phi), cos(phi) * cos(theta));
    float3 ro = CAMERA_POSITION;
    float max_dist = MAX_DIST;

    float2 planet_intersect = ray_sphere_intersect(ro - PLANET_CENTER, rd, PLANET_RADIUS - 100.0f);
    float groundDist = (planet_intersect.x > 0) ? planet_intersect.x : max_dist;
    float3 scene_color = dot(rd, LIGHT_DIRECTION) > 0.9998 ? 3.0 : 0.0;

    if (planet_intersect.y > 0.0f)
    {
        max_dist = max(planet_intersect.x, 0.0);
        scene_color = calculate_ground_scattering(ro, rd, planet_intersect,
            GROUND_COLOR, 3.0 * ATMO_RADIUS, ORIGIN, LIGHT_DIRECTION, float3(INTENSITY, INTENSITY, INTENSITY),
            PLANET_CENTER, PLANET_RADIUS, ATMO_RADIUS,
            RAYLEIGH_BETA, MIE_BETA, ABSORPTION_BETA, float3(AMBIENT_BETA, AMBIENT_BETA, AMBIENT_BETA),
            G, RAYLEIGH_HEIGHT, MIE_HEIGHT, ABSORPTION_HEIGHT, ABSORPTION_FALLOFF,
            GROUND_PRIMARY_STEPS, GROUND_LIGHT_STEPS);
    }

    float3 col = calculate_atmosphere_scattering(
        ro, rd, max_dist,
        scene_color,
        -LIGHT_DIRECTION,
        float3(INTENSITY, INTENSITY, INTENSITY),
        PLANET_CENTER,
        PLANET_RADIUS,
        ATMO_RADIUS,
        RAYLEIGH_BETA,
        MIE_BETA,
        ABSORPTION_BETA,
        float3(AMBIENT_BETA, AMBIENT_BETA, AMBIENT_BETA),
        G,
        RAYLEIGH_HEIGHT,
        MIE_HEIGHT,
        ABSORPTION_HEIGHT,
        ABSORPTION_FALLOFF,
        PRIMARY_STEPS,
        LIGHT_STEPS
    );

    OutTexture[DTid.xy] = float4(col, 1.0f);
} // main