// GodRayPS.hlsl
// https://www.shadertoy.com/view/ltcXDH
// https://www.shadertoy.com/view/ssV3zh
#include "Common.hlsli"
#include "PostProcess.hlsli"

SamplerState LinearSampler : register(s0);
Texture2D    InputTex : register(t0);
Texture2D    DepthTex : register(t1);
Texture2D    CloudTex : register(t2);

cbuffer GodRayBuffer : register(b2)
{
    float2 gLightUV;
    float  gDensity;
    float  gWeight;
    float  gDecay;
    float  gExposure;
    float  gLuminanceThreshold;
} // GodRayBuffer

#define LIGHT_UV  gLightUV
#define DENSITY   gDensity
#define WEIGHT    gWeight
#define DECAY     gDecay
#define EXPOSURE  gExposure
#define THRESHOLD gLuminanceThreshold
#define NUM_SAMPLES 24

float4 main(PS_INPUT input) : SV_TARGET
{
    float luminance = get_cross_luminance(InputTex, LinearSampler, LIGHT_UV, THRESHOLD);
    float cloudTransmittanceAtSun = CloudTex.SampleLevel(LinearSampler, LIGHT_UV, 0).a;
    
    if (luminance * cloudTransmittanceAtSun < 0.1f)
        return float4(0.0f, 0.0f, 0.0f, 0.0f);

    float2 deltaTexCoord = (input.uv - LIGHT_UV);
    deltaTexCoord *= 1.0f / (float) NUM_SAMPLES * DENSITY;

    float2 uv = input.uv;
    float3 color = float3(0.0f, 0.0f, 0.0f);
    float illuminationDecay = 1.0f;

    [loop]
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        uv -= deltaTexCoord;
        
        float cloudT = CloudTex.SampleLevel(LinearSampler, uv, 0).a;
        
        float depth = DepthTex.SampleLevel(LinearSampler, uv, 0).r;
        float isSky = (depth < 0.9999f) ? 1.0f : 0.0f;
        cloudT *= isSky;
        
        // 밝은 하이라이트(태양 근처) 추출
        float3 sampleColor = InputTex.SampleLevel(LinearSampler, uv, 0).rgb;
        float sampleLum = dot(sampleColor, float3(0.299f, 0.587f, 0.114f));
        
        sampleColor *= smoothstep(0.5f, 1.0f, sampleLum);

        float distToSun = length(uv - LIGHT_UV);
        float sunMask = saturate(1.0f - distToSun * 3.0f);
        
        sampleColor *= cloudT * sunMask;
        
        // 누적 및 감쇄
        color += sampleColor * illuminationDecay * WEIGHT;
        illuminationDecay *= DECAY;
    } // for

    return float4(color * EXPOSURE * luminance, 1.0f);
} // main