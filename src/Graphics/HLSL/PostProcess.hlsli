// PostProcess.hlsli
// https://www.shadertoy.com/view/4dSBDt
// https://www.shadertoy.com/view/Dtd3zl
// https://www.shadertoy.com/view/MdX3Rr
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

#endif // _POSTPROCESS_HLSLI_