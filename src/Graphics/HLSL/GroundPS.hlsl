// GroundPS.hlsl
#include "Common.hlsli"
#include "Maths.hlsli"
#include "ShadowMap.hlsli"
#include "Ground.hlsli"
#include "PBR.hlsli"

SamplerState           LinearSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D ObjectShadowMap : register(t10);
Texture2D TerrainShadowMap : register(t11);
Texture2D FarawayGrassTex : register(t12);
Texture2D AlbedoTex : register(t13);
Texture2D AmbientTex : register(t14);
Texture2D NormalTex : register(t15);
Texture2D RoughnessTex : register(t16);
Texture2D DisplacementTex : register(t17);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
}; // PS_IN

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

cbuffer GroundBuffer : register(b3)
{
    // [Row 1]
    float3 gDarkSand;
    float  gPadding1;
    // [Row 2]
    float3 gLightSand;
    float  gDist;
}; // GroundBuffer

#define WORLD               cWorld

#define COLOR_DARK_SAND     gDarkSand
#define COLOR_LIGHT_SAND    gLightSand
#define GROUND_DETAIL_DIST  gDist

float4 main(PS_IN input) : SV_TARGET
{
    float4 albedo = AlbedoTex.Sample(LinearSampler, input.uv);
    float roughness = RoughnessTex.Sample(LinearSampler, input.uv).r;
    float ao = AmbientTex.Sample(LinearSampler, input.uv).r;

    float3 normalSample = NormalTex.Sample(LinearSampler, input.uv).rgb * 2.0f - 1.0f;
    normalSample.xy *= 0.5f;
    normalSample = normalize(normalSample);

    float3x3 TBN = float3x3(
        normalize(input.tangent),
        normalize(input.binormal),
        normalize(input.normal)
    );
    float3 N = normalize(mul(normalSample, TBN));

    float3 L = normalize(-LIGHT_DIRECTION);
    float NdotL = saturate(dot(N, L));

    float3 lightColor = get_dynamic_light_color(LIGHT_DIRECTION.y).rgb;

    float4 lightClipPos = mul(mul(float4(input.worldPos, 1.0f), LIGHT_VIEW), LIGHT_PROJ);
    float4 objLightClipPos = mul(mul(float4(input.worldPos, 1.0f), LIGHT_OBJECT_VIEW), LIGHT_OBJECT_PROJ);

    float3 ndcPos = lightClipPos.xyz / lightClipPos.w;
    float terrainShadow = 1.0f, objectShadow = 1.0f;
    if (ndcPos.x >= -1.0f && ndcPos.x <= 1.0f &&
        ndcPos.y >= -1.0f && ndcPos.y <= 1.0f &&
        ndcPos.z >= 0.0f && ndcPos.z <= 1.0f)
    {
        terrainShadow = calculate_poisson_shadow(ShadowSampler, TerrainShadowMap, lightClipPos, SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);
        objectShadow = calculate_poisson_shadow(ShadowSampler, ObjectShadowMap, objLightClipPos, SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);
    }
    float shadowFactor = min(terrainShadow, objectShadow);

    float distToCamera = length(input.worldPos - CAMERA_POSITION);
    float2 tiledUV = input.worldPos.xz * 0.02f;
    float farBlend = smoothstep(GROUND_DETAIL_DIST * 1.5f, GROUND_DETAIL_DIST * 2.0f, distToCamera);

    float3 grassColor = FarawayGrassTex.Sample(LinearSampler, input.uv).rgb;
    float grassDiff = saturate(dot(normalize(input.normal), L));
    float3 farColor = grassColor * grassDiff * shadowFactor;

    if (farBlend >= 1.0f)
        return float4(farColor, 1.0f);

    float3 ambient = DEFAULT_AMBIENT.rgb * albedo.rgb * ao;
    float3 diffuse = albedo.rgb / PI;
    float3 radiance = lightColor * NdotL;

    float3 nearColor = diffuse * radiance * shadowFactor + ambient;

    float3 col = lerp(nearColor, farColor, farBlend);
    return float4(saturate(col), 1.0f);
} // main