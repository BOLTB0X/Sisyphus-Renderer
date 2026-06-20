// RefractionPS.hlsl
// https://www.rastertek.com/dx11win10tut31.html
#include "Common.hlsli"

SamplerState ClampSampler : register(s0);
Texture2D    RefractionRT : register(t0);

struct PS_IN
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float  clip : SV_ClipDistance0;
}; // PS_IN

float4 main(PS_IN input) : SV_TARGET
{	
    float4 textureColor = RefractionRT.Sample(ClampSampler, input.tex);
    float4 color = LIGHT_AMBIENT;

    float3 lightDir = -LIGHT_DIRECTION;

    float lightIntensity = saturate(dot(input.normal, lightDir));

    if (lightIntensity > 0.0f)
    {
        color += (get_dynamic_light_color(LIGHT_DIRECTION) * lightIntensity);
    }

    color = saturate(color);
    color = color * textureColor;
	
    return color;
} // main