// Noise.hlsli
// https://www.shadertoy.com/view/MdGfzh
// https://github.com/NadirRoGue/RenderEngine/tree/master/RenderEngine/RenderEngine/shaders/clouds/generation
#ifndef _NOISE_HLSLI_
#define _NOISE_HLSLI_

#include "Common.hlsli"
#include "Maths.hlsli"

float hash(int n)
{
    return frac(sin(float(n) + 1.951f) * 43758.5453123f);
} // hash

float hash_12(float2 p)
{
    p = 50.0 * frac(p * 0.3183099f);
    return frac(p.x * p.y * (p.x + p.y));
} // hash12

float hash_13(float3 p3)
{
    p3 = frac(p3 * 1031.1031f);
    p3 += dot(p3, p3.yzx + 19.19f);
    return frac((p3.x + p3.y) * p3.z);
} // hash13

float2 hash_22(float2 p)
{
    float n = sin(dot(p, float2(113.0f, 1.0f)));
    p = frac(float2(2097152.0f, 262144.0f) * n) * 2.0f - 1.0f;
    return p;
} // hash22

float3 hash_33(float3 p3)
{
    p3 = frac(p3 * 0.1031f);
    p3 += dot(p3, p3.yxz + 19.19f);
    return frac((p3.xxy + p3.yxx) * p3.zyx);
} // hash33

float noise(float3 x, float tile)
{
    float3 p = floor(x);
    float3 f = frac(x);

    f = f * f * (3.0f - 2.0f * f);
    
    return lerp(lerp(lerp(hash_33(fmod(p + float3(0, 0, 0), tile)),
                        hash_33(fmod(p + float3(1, 0, 0), tile)), f.x),
                   lerp(hash_33(fmod(p + float3(0, 1, 0), tile)),
                        hash_33(fmod(p + float3(1, 1, 0), tile)), f.x), f.y),
               lerp(lerp(hash_33(fmod(p + float3(0, 0, 1), tile)),
                        hash_33(fmod(p + float3(1, 0, 1), tile)), f.x),
                   lerp(hash_33(fmod(p + float3(0, 1, 1), tile)),
                        hash_33(fmod(p + float3(1, 1, 1), tile)), f.x), f.y), f.z);
} // noise

float voronoi(float3 x, float tile)
{
    float3 p = floor(x);
    float3 f = frac(x);

    float res = 100.;
    for (int k = -1; k <= 1; k++)
    {
        for (int j = -1; j <= 1; j++)
        {
            for (int i = -1; i <= 1; i++)
            {
                float3 b = float3(i, j, k);
                float3 c = p + b;

                if (tile > 0.)
                {
                    c = fmod(c, float3(tile, tile, tile));
                }

                float3 r = float3(b) - f + hash_13(c);
                float d = dot(r, r);

                if (d < res)
                {
                    res = d;
                }
            }
        }
    }

    return 1.0f - res;
} // voronoi

float tilable_voronoi(float3 p, const int octaves, float tile)
{
    float f = 1.;
    float a = 1.;
    float c = 0.;
    float w = 0.;

    if (tile > 0.)
        f = tile;

    for (int i = 0; i < octaves; i++)
    {
        c += a * voronoi(p * f, f);
        f *= 2.0;
        w += a;
        a *= 0.5;
    }

    return c / w;
} // tilablevoronoi

float tilable_fbm(float3 p, const int octaves, float tile)
{
    float f = 1.;
    float a = 1.;
    float c = 0.;
    float w = 0.;

    if (tile > 0.)
        f = tile;

    for (int i = 0; i < octaves; i++)
    {
        c += a * noise(p * f, f);
        f *= 2.0;
        w += a;
        a *= 0.5;
    }

    return c / w;
} // tilable_fbm

float grad_noise2D(float2 f)
{
    float2 e = float2(0.0, 1.0);
    float2 p = floor(f);
    f -= p;
    float2 w = f * f * (3.0 - 2.0 * f); // Cubic smoothing
    
    float c = lerp(
        lerp(dot(hash_22(p + e.xx), f - e.xx), dot(hash_22(p + e.yx), f - e.yx), w.x),
        lerp(dot(hash_22(p + e.xy), f - e.xy), dot(hash_22(p + e.yy), f - e.yy), w.x),
        w.y
    );
    return c * 0.5 + 0.5; // [0, 1] 범위로 정규화
} // grad_noise2D

float grad_wave(float x, float offset)
{
    x = abs(frac(x / 6.2831853 + offset - 0.25) - 0.5) * 2.0;
    float x2 = saturate(x * x * (-1.0 + 2.0 * x));
    x = smoothstep(0.0, 1.0, x);
    return lerp(x, x2, 0.15);
} // grad_wave


#endif // _NOISE_HLSLI_