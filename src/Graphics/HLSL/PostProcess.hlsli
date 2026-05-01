// PostProcess.hlsli
// https://www.shadertoy.com/view/4dSBDt
// https://www.shadertoy.com/view/Dtd3zl
// https://www.shadertoy.com/view/MdX3Rr
// https://www.shadertoy.com/view/4sX3Rs
// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
// https://github.com/chihirobelmo/volumetric-cloud-for-directx11/blob/main/VolumetricCloud/shaders/PostAA.hlsl
// https://github.com/NadirRoGue/RenderEngine/blob/master/RenderEngine/RenderEngine/shaders/postprocess/PostProcessRender.frag
// https://github.com/fede-vaccaro/TerrainEngine-OpenGL/blob/master/shaders/post_processing.frag
#ifndef _POSTPROCESS_HLSLI_
#define _POSTPROCESS_HLSLI_

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
}; // VS_OUT

static const int2 offsets_TAA[8] =
{
    int2(-1, -1), int2(-1, 1),
    int2(1, -1), int2(1, 1),
    int2(1, 0), int2(0, -1),
    int2(0, 1), int2(-1, 0)
}; // offsets

static const float g_color_box_sigma = 0.75f;

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
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
} // aces_film

float3 RGB_to_YCoCg(float3 RGB)
{
    float Y = dot(RGB, float3(1, 2, 1)) * 0.25f;
    float Co = dot(RGB, float3(2, 0, -2)) * 0.25f + (0.5f * 256.0f / 255.0f);
    float Cg = dot(RGB, float3(-1, 2, -1)) * 0.25f + (0.5f * 256.0f / 255.0f);
    return float3(Y, Co, Cg);
} // RGB_to_YCoCg

float3 YCoCg_to_RGB(float3 YCoCg)
{
    float Y = YCoCg.x;
    float Co = YCoCg.y - (0.5f * 256.0f / 255.0f);
    float Cg = YCoCg.z - (0.5f * 256.0f / 255.0f);
    return float3(Y + Co - Cg, Y + Cg, Y - Co - Cg);
} // YCoCg_to_RGB

float get_luminance(float3 color, float3 luminance)
{
    return dot(color, luminance);
} // getLuminance

float get_cross_luminance(Texture2D scene, SamplerState samp, float2 lightUV, float InterpolationOffest)
{
    if (lightUV.x < 0.0f || lightUV.x > 1.0f || lightUV.y < 0.0f || lightUV.y > 1.0f)
    {
        return 0.0f;
    }

    float2 offset = float2(0.005f, 0.005f);
    float3 c0 = scene.SampleLevel(samp, lightUV, 0).rgb;
    float3 c1 = scene.SampleLevel(samp, lightUV + float2(offset.x, 0), 0).rgb;
    float3 c2 = scene.SampleLevel(samp, lightUV - float2(offset.x, 0), 0).rgb;
    float3 c3 = scene.SampleLevel(samp, lightUV + float2(0, offset.y), 0).rgb;
    float3 c4 = scene.SampleLevel(samp, lightUV - float2(0, offset.y), 0).rgb;

    float3 avgColor = (c0 + c1 + c2 + c3 + c4) * 0.2f;
    float luminance = dot(avgColor, float3(0.299f, 0.587f, 0.114f)); // 밝기 계산
    
    return smoothstep(InterpolationOffest, 1.0f, luminance);
} // get_cross_luminance

#endif // _POSTPROCESS_HLSLI_