// GrassFarPS.hlsl
#include "Common.hlsli"
#include "ShadowMap.hlsli"
#include "PBR.hlsli"

SamplerState           LinearSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D GrassTex : register(t0);
Texture2D ObjectShadowMap : register(t10);
Texture2D TerrainShadowMap : register(t11);

struct PS_IN
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float  dist : TEXCOORD2;
    float3 rootWorldPos : TEXCOORD3;
}; // PS_IN

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

float4 main(PS_IN input, bool isFrontFace : SV_IsFrontFace) : SV_TARGET
{
    clip(input.dist - LIMIT_DIST);
    float mipLevel = clamp(log2(input.dist / 100.0f), 0.0f, 6.0f);
    float4 col = GrassTex.SampleLevel(LinearSampler, input.uv, mipLevel);

    clip(col.a - ALPHA_CUT);
    
    // 원경 빌보드는 위를 향하는 가짜 노멀 사용
    float3 N = float3(0, 1, 0);
    if (!isFrontFace)
    {
        N = -N;
    }
    
    float3 L = normalize(-LIGHT_DIRECTION);
    float3 V = normalize(CAMERA_POSITION - input.worldPos);
    float3 H = normalize(V + L);
    
    float3 lightColor = get_dynamic_light_color(LIGHT_DIRECTION.y).rgb;

    // 그림자 연산
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
    
    // PBR 매개변수 설정
    float heightFactor = saturate(input.uv.y);
    float ao = lerp(0.3f, 1.0f, 1.0f - heightFactor);
    float3 albedo = col.rgb;
    float roughness = 0.9f; // 원경은 디테일이 적어 조금 더 거칠게 세팅
    float metallic = 0.0f;
    
    // Cook-Torrance BRDF 연산
    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));
    float NdotH = saturate(dot(N, H));
    float HdotV = saturate(dot(H, V));

    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
    float3 F = F0 + (1.0f - F0) * pow(1.0f - HdotV, 5.0f);

    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = (NdotH * NdotH * (alpha2 - 1.0f) + 1.0f);
    float D = alpha2 / (PI * denom * denom);

    float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
    float G1 = NdotL / (NdotL * (1.0f - k) + k);
    float G2 = NdotV / (NdotV * (1.0f - k) + k);
    float G = G1 * G2;

    float3 numerator = D * G * F;
    float denominator = 4.0f * NdotL * NdotV + 0.0001f;
    float3 specular = numerator / denominator;

    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metallic;

    float3 directLightPBR = (kD * albedo / PI + specular) * lightColor * NdotL;

    // 잔디 특화: Translucency
    float wrap = 0.5f;
    float scatter = saturate((dot(-N, L) + wrap) / (1.0f + wrap));
    float3 translucency = albedo * lightColor * scatter * 0.4f * (1.0f - heightFactor);

    float3 finalLight = (directLightPBR + translucency) * finalShadow;
    float3 ambient = albedo * float3(0.15f, 0.15f, 0.2f) * ao;
    
    float3 finalColor = finalLight + ambient;

    return float4(finalColor, col.a);
} // main