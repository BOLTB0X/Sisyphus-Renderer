// RigidSampePS.hlsl
#include "Common.hlsli"
#include "PBR.hlsli"
#include "ShadowMap.hlsli"

SamplerState           PointSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D ObjectShadowMap : register(t10);
Texture2D TerrainShadowMap : register(t11);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : TEXCOORD;
}; // VS_OUT

float4 main(PS_IN input) : SV_TARGET
{
    float4 albedo = float4(0.6f, 0.6f, 0.6f, 1.0f);

    float3 N = normalize(input.normal);

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

    float3 diffuseColor = albedo.rgb;
    float3 ambient = DEFAULT_AMBIENT.rgb * albedo.rgb;
    
    float3 lit = diffuseColor * lightColor * NdotL * shadowFactor;
    float3 col = ambient + lit;

    return float4(saturate(col), albedo.a);
} // main