// TerrainPS.hlsl
#include "Common.hlsli"
#include "PBR.hlsli"
#include "ShadowMap.hlsli"

SamplerState           LinearSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D BaseNorTex : register(t2);
Texture2D SandTex : register(t3);
Texture2D GrassTex : register(t4);
Texture2D CliffTex : register(t5);
Texture2D SnowTex : register(t6);
Texture2D ObjectShadowMap : register(t10);
Texture2D TerrainShadowMap : register(t11);

struct PS_IN
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float  height : TEXCOORD1;
}; // PS_IN

struct PS_OUT
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
}; // PS_OUT

cbuffer TerrainBlendingBuffer : register(b5)
{
    float tWaterLevel;
    float tTransZone;
    float tGrassLimit;
    float tSnowHeight;

    float  tUVTiling;
    float3 tPadding;
}; // TerrainBlendingBuffer

#define WATER_LEVEL tWaterLevel
#define TRANS_ZONE  tTransZone
#define GRASS_LIMIT tGrassLimit
#define SNOW_HEIGHT tSnowHeight
#define UV_TILING   tUVTiling

PS_OUT main(PS_IN input) : SV_TARGET
{
    PS_OUT output;
    
    float2 tiledUV = input.uv * UV_TILING;
    float3 sandCol = SandTex.Sample(LinearSampler, tiledUV).rgb;
    float3 grassCol = GrassTex.Sample(LinearSampler, tiledUV).rgb;
    float3 cliffCol = CliffTex.Sample(LinearSampler, tiledUV).rgb;
    float3 snowCol = SnowTex.Sample(LinearSampler, tiledUV).rgb;

    float sandR = 0.8f, grassR = 0.9f, cliffR = 0.6f, snowR = 0.4f;

    float3 finalAlbedo;
    float finalRoughness;
    float slope = abs(input.normal.y);

    if (input.height <= WATER_LEVEL)
    {
        finalAlbedo = sandCol;
        finalRoughness = sandR;
    }
    else if (input.height <= WATER_LEVEL + TRANS_ZONE)
    {
        float t = (input.height - WATER_LEVEL) / TRANS_ZONE;
        finalAlbedo = lerp(sandCol, grassCol, t);
        finalRoughness = lerp(sandR, grassR, t);
    }
    else
    {
        if (slope > GRASS_LIMIT)
        {
            finalAlbedo = grassCol;
            finalRoughness = grassR;
        }
        else if (slope > GRASS_LIMIT - 0.15f)
        {
            float t = (slope - (GRASS_LIMIT - 0.15f)) / 0.15f;
            finalAlbedo = lerp(cliffCol, grassCol, t);
            finalRoughness = lerp(cliffR, grassR, t);
        }
        else
        {
            finalAlbedo = cliffCol;
            finalRoughness = cliffR;
        }
        
        if (input.height > SNOW_HEIGHT && slope > 0.6f)
        {
            float t = saturate((input.height - SNOW_HEIGHT) / 10.0f);
            finalAlbedo = lerp(finalAlbedo, snowCol, t);
            finalRoughness = lerp(finalRoughness, snowR, t);
        }
    }

    float3 N = normalize(input.normal);
    float3 V = normalize(CAMERA_POSITION - input.posW);
    float3 L = normalize(-LIGHT_DIRECTION);
    float3 H = normalize(V + L);

    float3 F0 = float3(0.04f, 0.04f, 0.04f); // 비금속 기본 반사율
    
    float NDF = distribution_GGX(N, H, finalRoughness);
    float G = geometry_smith(N, V, L, finalRoughness);
    float3 F = fresnel_schlick(saturate(dot(H, V)), F0);
    
    float3 spec = (NDF * G * F) / (4.0f * saturate(dot(N, V)) * saturate(dot(N, L)) + 0.001f);
    float3 kD = (1.0f - F) * (1.0f - 0.0f);

    float4 lightClip = mul(mul(float4(input.posW, 1.0f), LIGHT_VIEW), LIGHT_PROJ);
    float4 objLightClip = mul(mul(float4(input.posW, 1.0f), LIGHT_OBJECT_VIEW), LIGHT_OBJECT_PROJ);
    
    float terrainShadow = calculate_poisson_shadow(ShadowSampler, TerrainShadowMap, lightClip, 2048.0f, 1.0f, 0.005f);
    float objectShadow = calculate_poisson_shadow(ShadowSampler, ObjectShadowMap, objLightClip, 2048.0f, 1.0f, 0.005f);
    float shadowFactor = min(terrainShadow, objectShadow);

    float3 radiance = get_dynamic_light_color(LIGHT_DIRECTION.y).rgb;
    float NdotL = saturate(dot(N, L));
    float3 ambient = DEFAULT_AMBIENT.rgb * finalAlbedo;
    float3 diffuse = kD * finalAlbedo / PI;
    
    float3 color = ambient + (diffuse + spec) * radiance * NdotL * shadowFactor;

    output.color = float4(color, 1.0f);
    output.normal = float4(N * 0.5f + 0.5f, 1.0f);
    return output;
} // main
