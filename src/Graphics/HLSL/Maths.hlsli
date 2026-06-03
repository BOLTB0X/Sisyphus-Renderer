// Maths.hlsli
// https://www.shadertoy.com/view/Xttcz2
// https://www.shadertoy.com/view/ld3BzM
#ifndef _MATHS_HLSLI_
#define _MATHS_HLSLI_

float3 mod289(float3 x)
{
    return x - floor(x * (1.0f / 289.0f)) * 289.0f;
} // mod289

float4 mod289(float4 x)
{
    return x - floor(x * (1.0f / 289.0f)) * 289.0f;
} // mod289

float4 permute(float4 x)
{
    return mod289(((x * 34.0f) + 1.0f) * x);
} // permute

float4 taylorInv_sqrt(float4 r)
{
    return 1.79284291400159f - 0.85373472095314f * r;
} // taylorInvSqrt

float2x2 rot2(float a)
{
    float c = cos(a), s = sin(a);
    return float2x2(c, -s, s, c);
} // rot2

float4 fade(float4 t)
{
    return (t * t * t) * (t * (t * 6.0f - 15.0f) + 10.0f);
} // fade

float2 random2(float2 p)
{
    return frac(sin(float2(dot(p, float2(127.1f, 311.7f)),
                          dot(p, float2(269.5f, 183.3f)))) * 43758.5453f);
} // random2

float3x3 rotate_Y(float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return float3x3(
         c, 0, s,
         0, 1, 0,
        -s, 0, c
    );
} // rotate_Y

float get_random_rotation_plane(float2 plane)
{
    return frac(sin(dot(plane, float2(127.1f, 311.7f))) * 43758.5f) * 3.14159f;
} // get_random_rotation_plane

#endif // _MATHS_HLSLI_