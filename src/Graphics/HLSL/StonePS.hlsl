// StonePS.hlsl
#include "Common.hlsli"
#include "PBR.hlsli"

Texture2D    AlbedoTexture : register(t0);
Texture2D    NormalTexture : register(t1);
Texture2D    MetallicTexture : register(t2);
Texture2D    RoughnessTexture : register(t3);
Texture2D    AOTexture : register(t4);
SamplerState LinearSampler : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal   : NORMAL;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
}; // PS_INPUT

float4 main(PS_INPUT input) : SV_TARGET
{
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

    float3 col = (diffuse + specular) * radiance + ambient;

    return float4(saturate(col), albedo.a);
} // main