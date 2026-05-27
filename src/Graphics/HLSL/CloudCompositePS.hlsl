// CloudCompositePS.hlsl
// https://www.shadertoy.com/view/Dtd3zl
#include "PostProcess.hlsli"

SamplerState LinearSampler : register(s0);
Texture2D SceneTex : register(t0);
Texture2D CloudTex : register(t1);
Texture2D DepthTex : register(t2);

cbuffer ResolutionBuffer : register(b2)
{
    float2 rResolution;
    float2 rPadding;
} // ResolutionBuffer

#define RESOLUTION rResolution

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 cloud = CloudTex.Sample(LinearSampler, input.uv);
    float4 scene = SceneTex.Sample(LinearSampler, input.uv);
    float sceneDepth = DepthTex.Sample(LinearSampler, input.uv).r;

    float cloudBlend = smoothstep(0.9999f, 1.0f, sceneDepth);
    float3 skyCol = scene.rgb * cloud.a + cloud.rgb;
    float3 terrainCol = scene.rgb;
    float3 col = lerp(terrainCol, skyCol, cloudBlend);

    // 비네팅
    float2 uv = input.uv;
    float vignette = pow(16.0 * uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y), 0.1);
    col = lerp(col * col, col, vignette);

    // 필름 노이즈
    float noise = frac(sin(dot(input.uv, float2(127.1, 311.7))) * 43758.5453);
    col -= noise * 0.025;

    return float4(saturate(col), 1.0);
} // main