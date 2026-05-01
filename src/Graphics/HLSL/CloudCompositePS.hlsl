// CloudCompositePS.hlsl
// https://www.shadertoy.com/view/Dtd3zl
#include "PostProcess.hlsli"

cbuffer ResolutionBuffer : register(b2)
{
    float2 rResolution;
    float2 rPadding;
} // ResolutionBuffer

SamplerState LinearSampler : register(s0);
Texture2D    SceneTex : register(t0);
Texture2D    CloudTex : register(t1);

#define RESOLUTION rResolution

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 texelSize = RESOLUTION;
    
    //float4 cloud = sample_gaussian_blurred(CloudTex, LinearSampler, input.uv, texelSize);
    float4 cloud = CloudTex.Sample(LinearSampler, input.uv);
    float4 scene = SceneTex.Sample(LinearSampler, input.uv);
    
    float3 col = cloud.rgb + scene.rgb * cloud.a;
    
    // 비네팅
    float2 uv = input.uv;
    float vignette = pow(16.0 * uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y), 0.1);
    col = lerp(col * col, col, vignette);

    // 필름 노이즈
    float noise = frac(sin(dot(input.uv, float2(127.1, 311.7))) * 43758.5453);
    col -= noise * 0.025;

    return float4(saturate(col), 1.0);
} // main