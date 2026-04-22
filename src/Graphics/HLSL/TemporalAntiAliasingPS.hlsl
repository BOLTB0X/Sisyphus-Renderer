// TemporalAntiAliasingPS.hlsl
// https://www.shadertoy.com/view/4dSBDt
// https://www.moddb.com/mods/stalker-anomaly/addons/temporal-anti-aliasing
// https://github.com/fede-vaccaro/TerrainEngine-OpenGL/blob/master/shaders/copyFrame.frag
#include "PostProcess.hlsli"

cbuffer TAABuffer : register(b0)
{
    float  tBlendFactor;
    float3 tPadding;
    float2 tTexelSize;
    float2 tPadding2;
}; // TAABuffer

SamplerState LinearSampler : register(s0);
Texture2D    CurrentTex : register(t0);
Texture2D    HistoryTex : register(t1);

#define BLEND_FACTOR       tBlendFactor
#define TEXEL_SIZE         tTexelSize
#define G_COLORl_BOX_SIGMA g_color_box_sigma

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 current = RGB_to_YCoCg(CurrentTex.Sample(LinearSampler, input.uv).rgb);
    float3 history = RGB_to_YCoCg(HistoryTex.Sample(LinearSampler, input.uv).rgb);

    // Variance Clipping
    float3 colorAvg = current;
    float3 colorVar = current * current;

    [unroll]
    for (int i = 0; i < 8; i++)
    {
        float2 offsetUV = input.uv + float2(offsets_TAA[i]) * TEXEL_SIZE;
        float3 fetch = RGB_to_YCoCg(CurrentTex.Sample(LinearSampler, offsetUV).rgb);
        colorAvg += fetch;
        colorVar += fetch * fetch;
    }
    colorAvg /= 9.0f;
    colorVar /= 9.0f;

    float3 sigma = sqrt(max(float3(0, 0, 0), colorVar - colorAvg * colorAvg));
    float3 colorMin = colorAvg - G_COLORl_BOX_SIGMA * sigma;
    float3 colorMax = colorAvg + G_COLORl_BOX_SIGMA * sigma;

    history = clamp(history, colorMin, colorMax);

    float3 result = YCoCg_to_RGB(lerp(current, history, BLEND_FACTOR));
    return float4(result, 1.0f);
} // main