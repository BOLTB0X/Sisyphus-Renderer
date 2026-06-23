// WaterVS.hlsl
#include "Common.hlsli"

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
}; // VS_IN

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float2 uv : TEXCOORD0;
    float4 clipPos : TEXCOORD1;
}; // VS_OUT

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

#define WORLD           cWorld

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    float4 worldPos = mul(float4(input.pos, 1.0f), WORLD);
    output.worldPos = worldPos.xyz;
    
    output.pos = mul(worldPos, VIEW);
    output.pos = mul(output.pos, PROJ);
    
    output.uv = input.uv * 10.0f;
    
    output.clipPos = output.pos;
    
    return output;
} // main