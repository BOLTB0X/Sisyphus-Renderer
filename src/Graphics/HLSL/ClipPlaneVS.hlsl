// ClipPlaneVS.hlsl
// https://www.rastertek.com/dx11win10tut31.html
#include "Common.hlsli"

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
}; // VS_IN

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : TEXCOORD0;
    float2 uv : TEXCOORD1; 
    float  clipDistance : SV_ClipDistance0;
}; // VS_OUT

#define WORLD      cWorld


VS_OUT main(VS_IN input)
{
    VS_OUT output;

    float4 worldPos = mul(float4(input.pos, 1.0f), WORLD);
    output.pos = mul(worldPos, VIEW);
    output.pos = mul(output.pos, PROJ);

    output.worldPos = worldPos;
    output.uv = input.uv;
    output.clipDistance = dot(worldPos, CLIP_PLANE);
    
    return output;
} // main
