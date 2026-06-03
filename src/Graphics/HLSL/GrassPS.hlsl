// GrassPS.hlsl
#include "Common.hlsli"
#include "ShadowMap.hlsli"

SamplerState           LinearSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D GrassTex : register(t0);
Texture2D ObjectShadowMap : register(t10);
Texture2D TerrainShadowMap : register(t11);

struct GS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
    float3 rootWorldPos : TEXCOORD2;
}; // GS_OUT

cbuffer GrassBuffer : register(b3)
{
    float  gGrassWidth;
    float  gGrassHeight;
    float  gWindStrength;
    float  gWindSpeed;
    
    float  gDist;
    float  gAlphaCut;
    float2 gPadding;
}; // GrassBuffer

#define GRASS_WIDTH   gGrassWidth
#define GRASS_HEIGHT  gGrassHeight
#define WIND_STRENGTH gWindStrength
#define WIND_SPEED    gWindSpeed
#define LIMIT_DIST    gDist
#define ALPHA_CUT     gAlphaCut

float4 main(GS_OUT input, bool isFrontFace : SV_IsFrontFace) : SV_TARGET
{
    float4 col = GrassTex.Sample(LinearSampler, input.uv);
    clip(col.a - ALPHA_CUT);

    float3 N = normalize(input.normal);
    if (!isFrontFace)
    {
        N = -N;
    }
    
    float3 L = normalize(-LIGHT_DIRECTION);
    float3 lightColor = get_dynamic_light_color(LIGHT_DIRECTION.y).rgb;
    float NdotL = saturate(dot(N, L));
    
    float3 snappedPos = float3(input.worldPos.x, input.rootWorldPos.y, input.worldPos.z);

    float4 lightViewPos = mul(float4(snappedPos, 1.0f), LIGHT_VIEW);
    float4 lightClipPos = mul(lightViewPos, LIGHT_PROJ);
    
    float4 objLightViewPos = mul(float4(snappedPos, 1.0f), LIGHT_OBJECT_VIEW);
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
    
    float rawShadow = min(terrainShadow, objectShadow);
    float finalShadow = lerp(0.35f, 1.0f, rawShadow);

    float3 diff = lightColor * NdotL * finalShadow * 0.7f + 0.3f;
    
    float heightFactor = saturate(input.uv.y);
    float3 baseColor = col.rgb * lerp(0.4f, 1.0f, 1.0f - heightFactor);

    return float4(baseColor * diff, col.a);
} // main