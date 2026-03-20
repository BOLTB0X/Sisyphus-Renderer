// Maths.hlsl
// https://www.shadertoy.com/view/Xttcz2
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

float4 taylorInvSqrt(float4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
} // taylorInvSqrt
