// StoneVS.hlsl
#include "Common.hlsli"

struct VS_INPUT {
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    float3 binormal : BINORMAL;
}; // VS_INPUT

struct PS_INPUT {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal   : NORMAL;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
}; // PS_INPUT

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

#define WORLD cWorld

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    
    float4 worldPos = mul(float4(input.position, 1.0f), WORLD);
    output.worldPos = worldPos.xyz;
    output.position = mul(worldPos, VIEW);
    output.position = mul(output.position, PROJ);
    output.texCoord = input.texCoord;
    
    float3x3 worldMat3 = (float3x3) WORLD;
    output.normal = normalize(mul(input.normal, worldMat3));
    output.tangent = normalize(mul(input.tangent, worldMat3));
    output.binormal = normalize(mul(input.binormal, worldMat3));
    
    return output;
} // main