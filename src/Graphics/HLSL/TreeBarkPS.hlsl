// TreeBarkPS.hlsl
#include "Common.hlsli"
#include "PBR.hlsli"
#include "ShadowMap.hlsli"

SamplerState           LinearSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D AlbedoTex : register(t0);
Texture2D NormalTex : register(t1);
Texture2D SpecularTex : register(t2);
Texture2D OpacityTex : register(t3);
Texture2D LightingTex : register(t4);
Texture2D ObjectShadowMap : register(t10);
Texture2D TerrainShadow : register(t11);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
}; // PS_INPUT

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 albedo = AlbedoTex.Sample(LinearSampler, input.texCoord);
    float opacity = OpacityTex.Sample(LinearSampler, input.texCoord).r;

    clip(opacity - 0.5f);

    float3 normalMap = NormalTex.Sample(LinearSampler, input.texCoord).rgb;
    float3 specular = SpecularTex.Sample(LinearSampler, input.texCoord).rgb;
    float3 bakedLight = LightingTex.Sample(LinearSampler, input.texCoord).rgb;

    // 노말맵 적용
    float3 normalSample = normalMap * 2.0f - 1.0f;
    float3x3 TBN = float3x3(
        normalize(input.tangent),
        normalize(input.binormal),
        normalize(input.normal)
    );
    float3 N = normalize(mul(normalSample, TBN));

    float3 L = normalize(-LIGHT_DIRECTION);
    float NdotL = saturate(dot(N, L));

    float3 lightColor = get_dynamic_light_color(LIGHT_DIRECTION.y).rgb;

    float3 finalColor = albedo.rgb
        * (bakedLight * 0.5f + lightColor * NdotL * 0.5f)
        + specular * pow(saturate(dot(reflect(-L, N),
          normalize(CAMERA_POSITION - input.worldPos))), 16.0f);
    
    float4 lightViewPos = mul(float4(input.worldPos, 1.0f), LIGHT_VIEW);
    float4 lightClipPos = mul(lightViewPos, LIGHT_PROJ);
    float shadow = calculate_poisson_shadow(
        ShadowSampler, TerrainShadow, lightClipPos,
        SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);

    return float4(finalColor * shadow, 1.0f);
} // main