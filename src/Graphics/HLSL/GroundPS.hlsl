// GroundPS.hlsl
#include "Common.hlsli"
#include "Maths.hlsli"
#include "ShadowMap.hlsli"
#include "Ground.hlsli"
#include "PBR.hlsli"

SamplerState           LinearSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D AlbedoTex : register(t2);
Texture2D NormalTex : register(t3);
Texture2D ObjectShadowMap : register(t10);
Texture2D TerrainShadowMap : register(t11);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
}; // PS_IN

struct PS_OUT
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
}; // PS_OUT

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

#define WORLD               cWorld

static const float TILING_FACTOR = 50.0f;

PS_OUT main(PS_IN input) : SV_TARGET
{
    PS_OUT output;
    float4 albedo = AlbedoTex.Sample(LinearSampler, input.uv);

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
    
    if (ndcPos.x >= -1.0f && ndcPos.x <= 1.0f && ndcPos.y >= -1.0f && ndcPos.y <= 1.0f &&
        ndcPos.z >= 0.0f && ndcPos.z <= 1.0f)
    {
        terrainShadow = calculate_poisson_shadow(ShadowSampler, TerrainShadowMap, lightClipPos, SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);
    }

    float3 objNdcPos = objLightClipPos.xyz / objLightClipPos.w;

    if (objNdcPos.x >= -1.0f && objNdcPos.x <= 1.0f && objNdcPos.y >= -1.0f && objNdcPos.y <= 1.0f &&
        objNdcPos.z >= 0.0f && objNdcPos.z <= 1.0f)
    {
        objectShadow = calculate_poisson_shadow(ShadowSampler, ObjectShadowMap, objLightClipPos, SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);
    }
    float shadowFactor = min(terrainShadow, objectShadow);

    float3 ambient = DEFAULT_AMBIENT.rgb * albedo.rgb;
    float3 diffuse = albedo.rgb / PI;
    float3 radiance = lightColor * NdotL;

    float3 col = diffuse * radiance * shadowFactor + ambient;
    
    output.color = float4(saturate(col), 1.0f);
    output.normal = float4(normalize(input.normal) * 0.5f + 0.5f, 1.0f);
    return output;
} // main