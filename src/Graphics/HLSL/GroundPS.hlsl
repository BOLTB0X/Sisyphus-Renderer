// GroundPS.hlsl
#include "Common.hlsli"
#include "Maths.hlsli"
#include "FBM.hlsli"
#include "ShadowMap.hlsli"
#include "Ground.hlsli"

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
    float gPadding1;
    // [Row 2]
    float3 gLightSand;
    float gPadding2;
}; // GroundBuffer

cbuffer ShadowBuffer : register(b4)
{
    // [Row 1]
    matrix sShadowWorld;
    float sMapWidth;
    float sMapHeight;
    float sBias;
    float sSpread;
    // [Row 5]
    float4 sPadding;
}; // ShadowBuffer

Texture2D shadowMap : register(t10);
SamplerComparisonState shadowSampler : register(s5);

float4 main(PS_IN input) : SV_TARGET
{
    float distToCamera = length(input.worldPos - cCameraPosition);

    float sandPattern = get_sand_texture(input.worldPos.xz * 0.1, distToCamera);
    
    float3 baseColor = lerp(gDarkSand, gLightSand, sandPattern);
    
    float3 normal = float3(0, 1, 0);
    float diff = saturate(dot(normal, -cLightDirection));
    
    float4 lightViewPos = mul(float4(input.worldPos, 1.0f), cLightView);
    float4 lightClipPos = mul(lightViewPos, cLightProj);
    
    float2 shadowMapSize = float2(sMapWidth, sMapHeight);
    float shadowFactor = calculate_poisson_shadow(shadowSampler, shadowMap, lightClipPos, shadowMapSize, sSpread, sBias);

    return float4(baseColor * shadowFactor * diff, 1.0f);
} // main