// TreePS.hlsl
#include "Common.hlsli"
#include "PBR.hlsli"
#include "ShadowMap.hlsli"

SamplerState           LinearSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D AlbedoTex : register(t0);
Texture2D NormalTex : register(t1);
Texture2D RoughnessTex : register(t2);
Texture2D AlphaTex : register(t3);
Texture2D SubsurfaceTex : register(t4);
Texture2D ObjectShadowMap : register(t10);
Texture2D TerrainShadowMap : register(t11);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
}; // PS_INPUT

cbuffer CheckTransparentBuffer : register(b3)
{
    int    isLeaf;
    float3 padding;
}; // CheckTransparentBuffer

#define IS_LEAF isLeaf

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 albedo = AlbedoTex.Sample(LinearSampler, input.texCoord);
    float4 alphaSample = AlphaTex.Sample(LinearSampler, input.texCoord);
    
    float leafAlpha = (any(alphaSample)) ? alphaSample.r : albedo.a;

    float3 normalMap = NormalTex.Sample(LinearSampler, input.texCoord).rgb;
    float roughness = RoughnessTex.Sample(LinearSampler, input.texCoord).r;
    float sssMask = SubsurfaceTex.Sample(LinearSampler, input.texCoord).r;

    float3 normalSample = normalMap * 2.0f - 1.0f;
    float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.binormal), normalize(input.normal));
    float3 N = normalize(mul(normalSample, TBN));
    
    float3 V = normalize(CAMERA_POSITION - input.worldPos);
    float3 L = normalize(-LIGHT_DIRECTION);
    float3 H = normalize(V + L);
    float3 lightColor = get_dynamic_light_color(LIGHT_DIRECTION.y).rgb;

    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));

    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    float NDF = distribution_GGX(N, H, roughness);
    float G = geometry_smith(N, V, L, roughness);
    float3 F = fresnel_schlick(saturate(dot(H, V)), F0);

    float3 numerator = NDF * G * F;
    float denominator = 4.0f * max(NdotV, 0.001f) * max(NdotL, 0.001f);
    float3 specular = numerator / denominator;

    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;

    float3 finalColor = (kD * albedo.rgb / PI + specular) * lightColor * NdotL;
    
    if (IS_LEAF)
    {
        clip(leafAlpha - 0.3f);
        float3 sss = albedo.rgb * sssMask * saturate(dot(V, -L)) * lightColor * 2.0f;
        finalColor += sss;
    }

    float4 lightViewPos = mul(float4(input.worldPos, 1.0f), LIGHT_VIEW);
    float4 lightClipPos = mul(lightViewPos, LIGHT_PROJ);
    
    float4 objLightViewPos = mul(float4(input.worldPos, 1.0f), LIGHT_OBJECT_VIEW);
    float4 objLightClipPos = mul(objLightViewPos, LIGHT_OBJECT_PROJ);
    
    float terrainShadow = calculate_poisson_shadow(ShadowSampler, TerrainShadowMap, lightClipPos, SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);
    float objectShadow = calculate_poisson_shadow(ShadowSampler, ObjectShadowMap, objLightClipPos, SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);

    float shadowFactor = min(terrainShadow, objectShadow);
    
    return float4(finalColor * shadowFactor, 1.0f);
} // main