// CompositePS.hlsl
#include "PostProcess.hlsli"

SamplerState WrapSampler : register(s0);
Texture2D    BloomFlareTex : register(t0);
Texture2D    GodRayTex : register(t1);

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 scene = BloomFlareTex.Sample(WrapSampler, input.uv).rgb;
    float3 godRay = GodRayTex.Sample(WrapSampler, input.uv).rgb;

    return float4(scene + godRay, 1.0f);
} // main
