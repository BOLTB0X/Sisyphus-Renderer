// GroundVS.hlsl
#include "Common.hlsli"

struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
}; // VS_IN

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
}; // PS_INPUT

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

#define WORLD cWorld

PS_INPUT main(VS_IN input)
{
    PS_INPUT output;
    float4 wPos = mul(float4(input.pos, 1.0f), WORLD);
    output.worldPos = wPos.xyz;
    output.pos = mul(mul(wPos, VIEW), PROJ);
    
    output.normal = normalize(mul(input.normal, (float3x3) WORLD));
    output.uv = input.uv;
    return output;
} // main