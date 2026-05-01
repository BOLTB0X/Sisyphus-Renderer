// GodRayPS.hlsl
// https://www.shadertoy.com/view/ltcXDH
// https://www.shadertoy.com/view/ssV3zh
// https://godotshaders.com/shader/
// https://medium.com/community-play-3d/god-rays-whats-that-5a67f26aeac2
#include "Common.hlsli"
#include "PostProcess.hlsli"

SamplerState LinearSampler : register(s0);
Texture2D    InputTex : register(t0);
Texture2D    DepthTex : register(t1);
Texture2D    TransmittanceTex : register(t2);

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
    if (LIGHT_UV.x < 0.0f || LIGHT_UV.x > 1.0f ||
        LIGHT_UV.y < 0.0f || LIGHT_UV.y > 1.0f)
        return float4(0, 0, 0, 0);
    
    float luminance = get_cross_luminance(InputTex, LinearSampler, LIGHT_UV, THRESHOLD);
    float cloudTransmittanceAtSun = TransmittanceTex.SampleLevel(LinearSampler, LIGHT_UV, 0).r;

    float2 deltaTexCoord = (input.uv - LIGHT_UV);
    deltaTexCoord *= 1.0f / (float) NUM_SAMPLES * DENSITY;

    float2 uv = input.uv;
    float3 color = float3(0.0f, 0.0f, 0.0f);
    float illuminationDecay = 1.0f;

    [loop]
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        uv -= deltaTexCoord;
        
        float cloudT = TransmittanceTex.SampleLevel(LinearSampler, uv, 0).r;
        
        float depth = DepthTex.SampleLevel(LinearSampler, uv, 0).r;
        float isSky = (depth <= 0.0001f) ? 1.0f : 0.0f;
        cloudT *= isSky;
        
        float behindCloud = 1.0f - cloudT;
        
        // 밝은 하이라이트(태양 근처) 추출
        float3 sampleColor = InputTex.SampleLevel(LinearSampler, uv, 0).rgb;
        
        sampleColor *= cloudT;
        
        // 누적 및 감쇄
        color += sampleColor * illuminationDecay * WEIGHT;
        illuminationDecay *= DECAY;
    } // for

    return float4(get_dynamic_light_color(LIGHT_DIRECTION).rgb * color * EXPOSURE * luminance, 1.0f);
} // main