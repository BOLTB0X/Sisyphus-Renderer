// SkyBoxVS.hlsl
#include "Common.hlsli"

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

struct VS_IN
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
}; // VS_INPUT

struct PS_IN
{
    float4 position : SV_POSITION;
    float3 localPos : TEXCOORD0;
    float  clipDistance : SV_ClipDistance0;
}; // PS_INPUT

#define WORLD cWorld

PS_IN main(VS_IN input)
{
    PS_IN output;
    
    float4 worldPos = mul(input.position, WORLD);
    float4x4 viewNoTranslation = VIEW;
    viewNoTranslation._41_42_43 = 0.0f;
    
    float4 viewPos = mul(worldPos, viewNoTranslation);
    output.position = mul(viewPos, PROJ).xyww;
    output.position.z = output.position.w;
    output.localPos = input.position.xyz;
    output.clipDistance = dot(worldPos, CLIP_PLANE);
    return output;
} // main