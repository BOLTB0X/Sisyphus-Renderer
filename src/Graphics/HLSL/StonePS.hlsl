// StonePS.hlsl
#include "Common.hlsli"
#include "PBR.hlsli"
#include "ShadowMap.hlsli"

SamplerState           LinearSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D    AlbedoTexture : register(t0);
Texture2D    NormalTexture : register(t1);
Texture2D    MetallicTexture : register(t2);
Texture2D    RoughnessTexture : register(t3);
Texture2D    AOTexture : register(t4);
Texture2D    ObjectShadowMap : register(t10);
Texture2D    TerrainShadowMap : register(t11);

struct PS_IN
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal   : NORMAL;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
}; // PS_INPUT

struct PS_OUT
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
}; // PS_OUT

PS_OUT main(PS_IN input) : SV_TARGET
{
    PS_OUT output;
    float4 albedo = AlbedoTexture.Sample(LinearSampler, input.texCoord);
    float3 normalMap = NormalTexture.Sample(LinearSampler, input.texCoord).rgb;
    float metallic = MetallicTexture.Sample(LinearSampler, input.texCoord).r;
    float roughness = RoughnessTexture.Sample(LinearSampler, input.texCoord).r;
    float ao = AOTexture.Sample(LinearSampler, input.texCoord).r;

    float3 normalSample = normalMap * 2.0f - 1.0f;
    normalSample.xy *= 0.5f;
    normalSample = normalize(normalSample);
    
    float3x3 TBN = float3x3(
        normalize(input.tangent),
        normalize(input.binormal),
        normalize(input.normal)
    );
    float3 N = normalize(mul(normalSample, TBN));

    float3 V = normalize(CAMERA_POSITION - input.worldPos);
    float3 L = normalize(-LIGHT_DIRECTION);
    float3 H = normalize(V + L);

    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo.rgb, metallic);

    float NDF = distribution_GGX(N, H, roughness);
    float G = geometry_smith(N, V, L, roughness);
    float3 F = fresnel_schlick(saturate(dot(H, V)), F0);

    float3 numerator = NDF * G * F;
    float denominator = 4.0f * saturate(dot(N, V)) * saturate(dot(N, L)) + 0.001f;
    float3 specular = numerator / denominator;

    float3 kD = (1.0f - F) * (1.0f - metallic);

    float NdotL = saturate(dot(N, L));
    float3 lightColor = get_dynamic_light_color(LIGHT_DIRECTION.y).rgb;

    float3 ambient = DEFAULT_AMBIENT.rgb * albedo.rgb * ao;
    float3 diffuse = kD * albedo.rgb / PI;
    float3 radiance = lightColor * NdotL;
    
    float4 lightViewPos = mul(float4(input.worldPos, 1.0f), LIGHT_VIEW);
    float4 lightClipPos = mul(lightViewPos, LIGHT_PROJ);
    
    float4 objLightViewPos = mul(float4(input.worldPos, 1.0f), LIGHT_OBJECT_VIEW);
    float4 objLightClipPos = mul(objLightViewPos, LIGHT_OBJECT_PROJ);
    
    float terrainShadow = calculate_poisson_shadow(ShadowSampler, TerrainShadowMap, lightClipPos, SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);
    float objectShadow = calculate_poisson_shadow(ShadowSampler, ObjectShadowMap, objLightClipPos, SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);

    float shadowFactor = min(terrainShadow, objectShadow);
    float3 col = (diffuse + specular) * radiance * shadowFactor + ambient;

    //return float4(saturate(col), albedo.a);
    
    output.color = float4(saturate(col), albedo.a);
    output.normal = float4(normalize(input.normal) * 0.5f + 0.5f, 1.0f);
    return output;
} // main