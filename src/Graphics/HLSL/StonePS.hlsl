cbuffer LightBuffer : register(b1)
{
    float4 diffuseColor;
    float3 lightDirection;
    float padding;
}; // LightBuffer

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal   : NORMAL;
    float3 worldPos : TEXCOORD1;
}; // PS_INPUT

Texture2D    albedoTexture    : register(t0);
Texture2D    normalTexture    : register(t1);
Texture2D    metallicTexture  : register(t2);
Texture2D    roughnessTexture : register(t3);
Texture2D    aoTexture        : register(t4);
SamplerState sampler0      : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 textureColor = albedoTexture.Sample(sampler0, input.texCoord);
    float3 lightDir = normalize(-lightDirection);
    float3 normal = normalize(input.normal);
    float lightIntensity = saturate(dot(normal, lightDir));
    float4 ambient = float4(0.1f, 0.1f, 0.1f, 1.0f);
    float4 color = (textureColor * diffuseColor * lightIntensity) + (textureColor * ambient);
    float4 aoData = aoTexture.Sample(sampler0, input.texCoord);
    color.rgb *= aoData.r;

    return saturate(color);
} // main