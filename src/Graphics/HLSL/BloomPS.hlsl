// BloomPS.hlsl
// https://www.shadertoy.com/view/4dSBDt
#include "Common.hlsli"
#include "PostProcess.hlsli"
#include "Noise.hlsli"

SamplerState LinearSampler : register(s0);
Texture2D    InputTex : register(t0);

#define NUM_SAMPLES  20.0f
#define BLOOM_AMOUNT 0.05f

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 q = input.uv;
    float2 blurRadius = float2(20.0f, 20.0f) / SCREEN_RESOLUTION;
    
    float4 sum = float4(0, 0, 0, 0);
    float phiOffset = hash(dot((input.uv * SCREEN_RESOLUTION), float2(1.12f, 2.251f)) + TIME);
    
    [loop]
    for (float i = 0.0f; i < NUM_SAMPLES; i += 1.0f)
    {
        float2 r = blurRadius * i / NUM_SAMPLES;
        float phi = (i / NUM_SAMPLES + phiOffset) * 2.0f * PI;
        float2 uv = q + float2(sin(phi), cos(phi)) * r;
        sum += InputTex.SampleLevel(LinearSampler, uv, 0);
    }
    
    float3 original = InputTex.SampleLevel(LinearSampler, q, 0).rgb;
    float3 bloomed = lerp(original, sum.rgb / NUM_SAMPLES, BLOOM_AMOUNT);
    
    // 노출 + ACES 톤매핑
    float exposure = 0.7f * (1.0f + 0.2f * sin(0.5f) * sin(1.8f));
    float3 col = aces_film(exposure * bloomed);
    
    return float4(col, 1.0f);
} // main