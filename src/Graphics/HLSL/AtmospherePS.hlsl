// AtmospherePS.hlsl
cbuffer LightBuffer : register(b1)
{
    float4 diffuseColor;
    float3 lightDirection;
    float  padding;
}; // LightBuffer

cbuffer AtmosphereBuffer : register(b2)
{
    float4 zenithColor; // 정점(하늘 위쪽) 색상
    float4 horizonColor; // 지평선 색상
}; // AtmosphereBuffer

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITIONT;
    float2 texCoord : TEXCOORD0;
}; // PS_INPUT

float4 main(PS_INPUT input) : SV_TARGET
{
    float height = input.texCoord.y;
    
    if (height < 0.0f)
    {
        height = 0.0f;
    }
 
    return lerp(horizonColor, zenithColor, height);
} // main