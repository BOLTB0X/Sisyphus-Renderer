// LensflarePS.hlsl
// https://www.shadertoy.com/view/4sX3Rs
// https://john-chapman-graphics.blogspot.com/2013/02/pseudo-lens-flare.html
// https://github.com/BOLTB0X/DirectX11-Draw/blob/main/SisyphusEngine/src/HLSL/LensFlarePS.hlsl
#include "Common.hlsli"
#include "PostProcess.hlsli"
#include "Noise.hlsli"

SamplerState LinearWrapSampler : register(s0);
Texture2D    BloomTex : register(t0);
Texture2D    NoiseTex : register(t1);
Texture2D    CompositeTex : register(t2);
Texture2D    CloudTex : register(t3);

cbuffer LensFlareBuffer : register(b2)
{
    float3 lRetouchColor;
    float  lNoiseScale;
    
    float2 lCoreUV;
    float  lCoreTight;
    float  lLuminanceOffset;
} // LensFlareBuffer

#define RETOUCH_COLOR    lRetouchColor
#define NOISE_SCALE      lNoiseScale
#define SUN_UV           lCoreUV
#define SUN_TIGHT        lCoreTight
#define LUMINANCE_OFFSET lLuminanceOffset

float3 Lensflare(float2 uv, float2 pos)
{
    float2 main = uv - pos;
    float2 uvd = uv * length(uv);
    
    float ang = atan2(main.x, main.y);
    float dist = length(main);
    dist = pow(abs(dist), 0.1f);
    
    // 링, halo
    float f1 = max(0.01f - pow(abs(length(uv + 1.2f * pos)), 1.9f), 0.0f) * 7.0f;
    float f2 = max(1.0f / (1.0f + 32.0f * pow(abs(length(uvd + 0.8f * pos)), 2.0f)), 0.0f) * 0.25f;
    float f22 = max(1.0f / (1.0f + 32.0f * pow(abs(length(uvd + 0.85f * pos)), 2.0f)), 0.0f) * 0.23f;
    float f23 = max(1.0f / (1.0f + 32.0f * pow(abs(length(uvd + 0.9f * pos)), 2.0f)), 0.0f) * 0.21f;
    
    // Ghosts
    float2 uvx = lerp(uv, uvd, -0.5f);
    float f4 = max(0.01f - pow(abs(length(uvx + 0.4f * pos)), 2.4f), 0.0f) * 6.0f;
    float f42 = max(0.01f - pow(abs(length(uvx + 0.45f * pos)), 2.4f), 0.0f) * 5.0f;
    float f43 = max(0.01f - pow(abs(length(uvx + 0.5f * pos)), 2.4f), 0.0f) * 3.0f;
    
    uvx = lerp(uv, uvd, -0.4f);
    float f5 = max(0.01f - pow(abs(length(uvx + 0.2f * pos)), 5.5f), 0.0f) * 2.0f;
    float f52 = max(0.01f - pow(abs(length(uvx + 0.4f * pos)), 5.5f), 0.0f) * 2.0f;
    float f53 = max(0.01f - pow(abs(length(uvx + 0.6f * pos)), 5.5f), 0.0f) * 2.0f;
    
    uvx = lerp(uv, uvd, -0.5f);
    float f6 = max(0.01f - pow(abs(length(uvx - 0.3f * pos)), 1.6f), 0.0f) * 6.0f;
    float f62 = max(0.01f - pow(abs(length(uvx - 0.325f * pos)), 1.6f), 0.0f) * 3.0f;
    float f63 = max(0.01f - pow(abs(length(uvx - 0.35f * pos)), 1.6f), 0.0f) * 5.0f;
    
    float3 c = float3(0.0f, 0.0f, 0.0f);
    c.r += f2 + f4 + f5 + f6;
    c.g += f22 + f42 + f52 + f62;
    c.b += f23 + f43 + f53 + f63;
    c = c * 1.3f - float3(length(uvd) * 0.05f, length(uvd) * 0.05f, length(uvd) * 0.05f);
    
    return c;
} // Lensflare

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 baseColor = BloomTex.SampleLevel(LinearWrapSampler, input.uv, 0).rgb;
    float aspectRatio = SCREEN_RESOLUTION.x / SCREEN_RESOLUTION.y;
    
    float2 uv = input.uv - 0.5f;
    uv.x *= aspectRatio;
    
    float2 sunPos = SUN_UV - 0.5f;
    sunPos.x *= aspectRatio;
    
    float luminance = get_cross_luminance(CompositeTex, LinearWrapSampler, SUN_UV, LUMINANCE_OFFSET);
    float cloudTransmittance = CloudTex.SampleLevel(LinearWrapSampler, SUN_UV, 0).a;
    
    if (luminance * cloudTransmittance < 0.2f)
    {
        return float4(baseColor, 1.0f);
    }

    float3 flareColor = RETOUCH_COLOR * Lensflare(uv, sunPos);
    
    flareColor -= noise_texture(NoiseTex, LinearWrapSampler, input.uv * SCREEN_RESOLUTION) * NOISE_SCALE;
    return float4(baseColor + flareColor, 1.0f);
} // main