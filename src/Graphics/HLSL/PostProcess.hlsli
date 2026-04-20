// PostProcess.hlsli
#ifndef _POSTPROCESS_HLSLI_
#define _POSTPROCESS_HLSLI_

float4 sample_cloud_blurred(Texture2D tex, SamplerState samp, float2 uv, float2 texelSize)
{
    float4 col = float4(0, 0, 0, 0);
    float weights[3] = { 0.25f, 0.5f, 0.25f };
    float2 offsets[3] = { float2(-1, 0), float2(0, 0), float2(1, 0) };
    
    for (int i = 0; i < 3; i++)
    {
        col += tex.Sample(samp, uv + offsets[i] * texelSize) * weights[i];
    }
    return col;
} // sample_cloud_blurred

// 5x5 Gaussian
float4 sample_gaussian_blurred(Texture2D tex, SamplerState samp, float2 uv, float2 texelSize)
{
    static const float kernel[5] = { 0.0625, 0.25, 0.375, 0.25, 0.0625 };
    float4 result = 0;
    [unroll]
    for (int x = -2; x <= 2; x++)
    [unroll]
        for (int y = -2; y <= 2; y++)
        {
            float2 offset = float2(x, y) * texelSize;
            result += tex.Sample(samp, uv + offset)
                  * kernel[x + 2] * kernel[y + 2];
        }
    return result;
} // sample_gaussian_blurred

float3 aces_film(float3 x)
{
    float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
} // aces_film

#endif // _POSTPROCESS_HLSLI_