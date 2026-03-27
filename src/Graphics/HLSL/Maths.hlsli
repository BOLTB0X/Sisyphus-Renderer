// Maths.hlsli
// https://www.shadertoy.com/view/Xttcz2
// https://www.shadertoy.com/view/ld3BzM
#ifndef _MATHS_HLSLI_
#define _MATHS_HLSLI_

float3 mod289(float3 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
} // mod289

float4 mod289(float4 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
} // mod289

float4 permute(float4 x)
{
    return mod289(((x * 34.0) + 1.0) * x);
} // permute

float4 taylorInv_sqrt(float4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
} // taylorInvSqrt

float2x2 rot2(float a)
{
    float c = cos(a), s = sin(a);
    return float2x2(c, -s, s, c);
} // rot2

#endif // _MATHS_HLSLI_