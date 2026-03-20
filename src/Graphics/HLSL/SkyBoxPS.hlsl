// SkyBoxPS.hlsl
// GroundColor 부분 참고 https://www.shadertoy.com/view/wlBXWK
//#include "Raymarching.hlsl"
#include "Atmosphere.hlsl"

SamplerState LinerSampler : register(s0);
Texture2D SceneDepthTexture : register(t1);

cbuffer CommonBuffer : register(b0)
{
    // [Row 1]
    matrix cWorld;
    // [Row 2]
    matrix cView;
    // [Row 3]
    matrix cProj;
    // [Row 4]
    float3 cCameraPosition;
    float  cPadding1;
    // [Row 5]
    matrix cViewInv;
    // [Row 6]
    matrix cProjInv;
    // [Row 7]
    float3 cLightDirection;
    float  cPadding2;
    // [Row 8]
    float4 cLightDiffuse;
    // [Row 9]
    float2 cResolution;
}; // CommonBuffer

cbuffer AtmosphereBuffer : register(b1)
{
    // [Row 1] 단순 그라데이션
    float4 aZenithColor;
    // [Row 2] 단순 그라데이션
    float4 aHorizonColor;
    // [Row 3] 행성 물리 데이터
    float3 aPlanetCenter;
    float aPlanetRadius;
    // [Row 4] 대기권 물리 데이터
    float aAtmoRadius;
    float3 aPadding;
    // [Row 5] 산란 계수 (Rayleigh)
    float3 aRayleighBeta;
    float aMieBeta;
    // [Row 6] 흡수 및 주변광
    float3 aAbsorptionBeta;
    float aAmbientBeta;
    // [Row 7] 고도 상수 (Density Falloff)
    float aRayleighHeight;
    float aMieHeight;
    float aAbsorptionHeight;
    float aAbsorptionFalloff;
    // [Row 8] Mie 위상 함수 및 샘플링 설정
    float aG;
    int aPrimarySteps;
    int aLightSteps;
    float aIntensity;
    // [Row 9] 지표면 색상
    float3 aGroundColor;
    float aPadding2;
    // [Row 10] 지표면 레이마칭 설정
    int aGroundPrimarySteps;
    int agroundLightSteps;
    float aPadding3;
}; // AtmosphereBuffer

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 localPos : TEXCOORD0;
}; // PS_INPUT

float3 CalculateGroundColor(float3 ro, float3 rd, float2 planet_intersect)
{
    float3 final_ground_color = aGroundColor;
    
    // 히트 지점 계산
    float t_hit = max(planet_intersect.x, 0.0);
    float3 hit_pos = ro + rd * t_hit;
    float3 surface_normal = normalize(hit_pos - aPlanetCenter);
        
    // 그림자 및 라이팅 계산
    float3 L = -cLightDirection;
    float3 V = -rd;
    float dotNL = max(MIN_DIST, dot(surface_normal, L));
    float dotNV = max(MIN_DIST, dot(surface_normal, V));
    float shadow = dotNL / (dotNL + dotNV);
    
    final_ground_color *= shadow;
    
    float3 bent_normal = normalize(lerp(surface_normal, L, 0.6));

    // 지표면에서 하늘 방향으로의 간접광 산란
    float3 sky_ambient = calculate_scattering(
            hit_pos, bent_normal, 3.0 * aAtmoRadius,
            ORIGIN, L, float3(aIntensity, aIntensity, aIntensity),
            aPlanetCenter, aPlanetRadius, aAtmoRadius,
            aRayleighBeta, aMieBeta, aAbsorptionBeta, float3(aAmbientBeta, aAmbientBeta, aAmbientBeta),
            aG, aRayleighHeight, aMieHeight, aAbsorptionHeight, aAbsorptionFalloff,
            aGroundPrimarySteps, agroundLightSteps
    );

    final_ground_color += clamp(sky_ambient * aGroundColor, 0.0, 1.0);
    
    return final_ground_color;
} // CalculateGroundColor

float3 GetWorldPosFromDepth(float2 uv, float depth)
{
    float4 clipPos;
    clipPos.x = uv.x * 2.0f - 1.0f;
    clipPos.y = (1.0f - uv.y) * 2.0f - 1.0f;
    clipPos.z = depth;
    clipPos.w = 1.0f;

    float4 worldPos = mul(clipPos, cProjInv);
    worldPos /= worldPos.w;
    worldPos = mul(worldPos, cViewInv);

    return worldPos.xyz;
} // GetWorldPosFromDepth

float4 main(PS_INPUT input) : SV_TARGET
{  
    //return float4(1.0f, 0.0f, 0.0f, 1.0f);
    float3 rd = normalize(input.localPos); // 시선 방향
    float3 ro = cCameraPosition / 1000.0f; // 카메라 위치
    float2 uv = input.position.xy / float2(cResolution.x, cResolution.y);
    float depth = SceneDepthTexture.Load(int3(input.position.xy, 0)).r;

    float max_dist = MAX_DIST;

    if (depth < 1.0f)
    {
        float3 worldPos = GetWorldPosFromDepth(uv, depth);
        max_dist = length(worldPos - cCameraPosition) / 1000.0f;
    }
    
    float2 planet_intersect = ray_sphere_intersect(ro - aPlanetCenter, rd, aPlanetRadius);
    float3 scene_color = dot(rd, cLightDirection) > 0.9998 ? 3.0 : 0.0;
    
    if (planet_intersect.y > 0.0)
    {
        max_dist = max(planet_intersect.x, 0.0);
        scene_color = CalculateGroundColor(ro, rd, planet_intersect);
    }
    
    if (depth >= 1.0f && planet_intersect.y <= 0.0)
    {
        scene_color = sun_color(rd, -cLightDirection, aIntensity);
    }

    float3 col = calculate_scattering(
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
    
    col = 1.0 - exp(-1.0 * col);
    return float4(col, 1.0f);
} // main