// GroundPS.hlsl
#include "Common.hlsli"
#include "Maths.hlsli"
#include "ShadowMap.hlsli"
#include "Ground.hlsli"

SamplerState           LinearSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D ObjectShadowMap : register(t10);
Texture2D TerrainShadowMap : register(t11);
Texture2D GroundTex : register(t12);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
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
    float distToCamera = length(input.worldPos - CAMERA_POSITION);
    float2 tiledUV = input.worldPos.xz * 0.02f; // 타일링
    float4 groundTex = GroundTex.Sample(LinearSampler, tiledUV);
    float grassBlend = smoothstep(GROUND_DETAIL_DIST * 2.0f, GROUND_DETAIL_DIST * 1.5f, distToCamera);
    float3 baseColor = lerp(groundTex.rgb, COLOR_DARK_SAND, grassBlend);
    
    float3 normal = normalize(input.normal);
    float diff = saturate(dot(normal, -LIGHT_DIRECTION));
    
    float4 lightViewPos = mul(float4(input.worldPos, 1.0f), LIGHT_VIEW);
    float4 lightClipPos = mul(lightViewPos, LIGHT_PROJ);
    
    float4 objLightViewPos = mul(float4(input.worldPos, 1.0f), LIGHT_OBJECT_VIEW);
    float4 objLightClipPos = mul(objLightViewPos, LIGHT_OBJECT_PROJ);
    
    float3 ndcPos = lightClipPos.xyz / lightClipPos.w;
    
    float terrainShadow = 1.0f;
    float objectShadow = 1.0f;

    if (ndcPos.x >= -1.0f && ndcPos.x <= 1.0f &&
    ndcPos.y >= -1.0f && ndcPos.y <= 1.0f &&
    ndcPos.z >= 0.0f && ndcPos.z <= 1.0f)
    {
        terrainShadow = calculate_poisson_shadow(ShadowSampler, TerrainShadowMap, lightClipPos, SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);
        objectShadow = calculate_poisson_shadow(ShadowSampler, ObjectShadowMap, objLightClipPos, SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);
    }
    
    float shadowFactor = min(terrainShadow, objectShadow);
    float lighting = saturate(diff * shadowFactor);
    
    return float4(baseColor * lighting, 1.0f);
} // main