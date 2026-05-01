// GroundPS.hlsl
#include "Common.hlsli"
#include "Maths.hlsli"
#include "ShadowMap.hlsli"
#include "Ground.hlsli"

SamplerComparisonState shadowSampler : register(s5);
Texture2D shadowMap : register(t10);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
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
    float gPadding2;
}; // GroundBuffer

cbuffer ShadowBuffer : register(b4)
{
    // [Row 1 ~ 2]
    matrix sShadowWorld;
    // [Row 3]
    float  sMapWidth;
    float  sMapHeight;
    float  sBias;
    // [Row 4]
    float  sSpread;
    float4 sPadding;
}; // ShadowBuffer

#define WORLD               cWorld

#define COLOR_DARK_SAND     gDarkSand
#define COLOR_LIGHT_SAND    gLightSand

#define SHADOW_WORLD        sShadowWorld
#define SHADOW_MAP_SIZE     float2(sMapWidth, sMapHeight)
#define SHADOW_BIAS         sBias
#define SHADOW_SPREAD       sSpread

float4 main(PS_IN input) : SV_TARGET
{
    float distToCamera = length(input.worldPos - CAMERA_POSITION);

    float sandPattern = get_sand_texture(input.worldPos.xz * 0.1, distToCamera);
    
    float3 baseColor = lerp(COLOR_DARK_SAND, COLOR_LIGHT_SAND, sandPattern);
    
    float3 normal = float3(0, 1, 0);
    float diff = saturate(dot(normal, -LIGHT_DIRECTION));
    
    float4 lightViewPos = mul(float4(input.worldPos, 1.0f), LIGHT_VIEW);
    float4 lightClipPos = mul(lightViewPos, LIGHT_PROJ);
    
    float2 shadowMapSize = SHADOW_MAP_SIZE;
    float shadowFactor = calculate_poisson_shadow(shadowSampler, shadowMap, lightClipPos, shadowMapSize, SHADOW_SPREAD, SHADOW_BIAS);

    return float4(baseColor * shadowFactor * diff, 1.0f);
} // main