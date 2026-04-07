// GroundVS.hlsl
#include "Common.hlsli"

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
}; // VS_IN

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float2 uv : TEXCOORD;
}; // PS_INPUT

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

PS_INPUT main(VS_IN input)
{
    PS_INPUT output;
    float4 wPos = mul(float4(input.pos, 1.0f), cWorld);
    output.worldPos = wPos.xyz;
    output.pos = mul(mul(wPos, cView), cProjection);
    output.uv = input.uv;
    return output;
} // main