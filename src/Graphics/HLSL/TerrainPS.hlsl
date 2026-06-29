// TerrainPS.hlsl
#include "Common.hlsli"
#include "ShadowMap.hlsli"

SamplerState           LinearSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D BaseColTex : register(t2);
Texture2D BaseNorTex : register(t3);
Texture2D SandTex : register(t4);
Texture2D GrassTex : register(t5);
Texture2D CliffTex : register(t6);
Texture2D SnowTex : register(t7);
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
    float  tWaterLevel;
    float  tTransZone;
    float  tGrassLimit;
    float  tSnowHeight;

    float  tUVTiling;
    float3 tPadding;
}; // HeightScaleBuffer

#define WATER_LEVEL tWaterLevel
#define TRANS_ZONE  tTransZone
#define GRASS_LIMIT tGrassLimit
#define SNOW_HEIGHT tSnowHeight
#define UV_TILING   tUVTiling

PS_OUT main(PS_IN input) : SV_TARGET
{
    PS_OUT output;
    
    // 지형 텍스처 블렌딩
    float2 tiledUV = input.uv * UV_TILING;
    float3 sandColor = SandTex.Sample(LinearSampler, tiledUV).rgb;
    float3 grassColor = GrassTex.Sample(LinearSampler, tiledUV).rgb;
    float3 cliffColor = CliffTex.Sample(LinearSampler, tiledUV).rgb;
    float3 snowColor = SnowTex.Sample(LinearSampler, tiledUV).rgb;

    float3 finalAlbedo = grassColor;
    float slope = abs(input.normal.y);
    
    if (input.height <= WATER_LEVEL)
        finalAlbedo = sandColor;
    else if (input.height <= WATER_LEVEL + TRANS_ZONE)
        finalAlbedo = lerp(sandColor, grassColor, (input.height - WATER_LEVEL) / TRANS_ZONE);
    else
    {
        if (slope > GRASS_LIMIT)
            finalAlbedo = grassColor;
        else if (slope > GRASS_LIMIT - 0.15f)
            finalAlbedo = lerp(cliffColor, grassColor, (slope - (GRASS_LIMIT - 0.15f)) / 0.15f);
        else
            finalAlbedo = cliffColor;
        
        if (input.height > SNOW_HEIGHT && slope > 0.6f)
            finalAlbedo = lerp(finalAlbedo, snowColor, saturate((input.height - SNOW_HEIGHT) / 10.0f));
    }

    // 쉐도우 연산
    float4 lightClipPos = mul(float4(input.posW, 1.0f), LIGHT_VIEW);
    lightClipPos = mul(lightClipPos, LIGHT_PROJ);

    float4 objLightClipPos = mul(float4(input.posW, 1.0f), LIGHT_OBJECT_VIEW);
    objLightClipPos = mul(objLightClipPos, LIGHT_OBJECT_PROJ);

    float3 ndcPos = lightClipPos.xyz / lightClipPos.w;
    float terrainShadow = 1.0f, objectShadow = 1.0f;

    // 지형 그림자 샘플링
    if (ndcPos.x >= -1.0f && ndcPos.x <= 1.0f && ndcPos.y >= -1.0f && ndcPos.y <= 1.0f && ndcPos.z >= 0.0f && ndcPos.z <= 1.0f)
    {
        terrainShadow = calculate_poisson_shadow(ShadowSampler, TerrainShadowMap, lightClipPos, 2048.0f, 1.0f, 0.005f);
    }

    // 객체 그림자 샘플링
    float3 objNdcPos = objLightClipPos.xyz / objLightClipPos.w;
    if (objNdcPos.x >= -1.0f && objNdcPos.x <= 1.0f && objNdcPos.y >= -1.0f && objNdcPos.y <= 1.0f && objNdcPos.z >= 0.0f && objNdcPos.z <= 1.0f)
    {
        objectShadow = calculate_poisson_shadow(ShadowSampler, ObjectShadowMap, objLightClipPos, 2048.0f, 1.0f, 0.005f);
    }
    
    float shadowFactor = min(terrainShadow, objectShadow);
    float3 lightDir = normalize(-LIGHT_DIRECTION);
    float NdotL = saturate(dot(input.normal, lightDir));
    
    float3 ambient = 0.2f * LIGHT_COLOR.rgb * finalAlbedo;
    float3 diffuse = shadowFactor * NdotL * LIGHT_COLOR.rgb * finalAlbedo; // diffuse에 적용
    
    output.color = float4(ambient + diffuse, 1.0f);
    output.normal = float4(normalize(input.normal) * 0.5f + 0.5f, 1.0f);
    
    return output;
} // main