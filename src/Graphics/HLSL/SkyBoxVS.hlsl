// SkyBoxVS.hlsl
#include "Common.hlsli"

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

struct VS_INPUT
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
}; // VS_INPUT

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 localPos : TEXCOORD0;
}; // PS_INPUT

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    
    float4 worldPos = mul(input.position, cWorld);
    float4x4 viewNoTranslation = cView;
    viewNoTranslation._41_42_43 = 0.0f;
    
    float4 viewPos = mul(worldPos, viewNoTranslation);
    output.position = mul(viewPos, cProjection).xyww;
    //output.position = mul(viewPos, cProjection);
    output.position.z = output.position.w;
    output.localPos = input.position.xyz;
    return output;
} // main