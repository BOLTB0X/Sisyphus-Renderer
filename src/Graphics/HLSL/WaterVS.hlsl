// WaterVS.hlsl
// https://www.rastertek.com/dx11win10tut31.html
#include "Common.hlsli"

cbuffer WaterBuffer : register(b2)
{
    matrix wWorldMatrix;
    
    float  waterHeight;
    float3 wPadding1;
    
    float3 waterColorShallow;
    float  wPadding2;
    
    float3 waterColorDeep;
    float  wPadding3;
    
    float  distortion;
    float  reflectivity;
    float  density;
    float  wPadding4;
}; // WaterBuffer

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
}; // VS_IN

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 clipSpace : TEXCOORD1;
}; // VS_OUT

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    
    float4 worldPos = mul(float4(input.pos, 1.0f), wWorldMatrix);
    output.pos = mul(worldPos, VIEW);
    output.pos = mul(output.pos, PROJ);
    
    output.uv = input.uv;
    output.clipSpace = output.pos;
    
    return output;
} // main