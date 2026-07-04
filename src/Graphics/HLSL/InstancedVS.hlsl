// InstancedVS.hlsl
#include "Common.hlsli"

struct InstanceData
{
    float3 position;
    float  scale;
    float3 normal;
    float  type;
}; // InstanceData

StructuredBuffer<InstanceData> TreeInstanceBuffer : register(t5);

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;

    uint  instanceID : SV_InstanceID;
}; // VS_IN

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
}; // VS_OUT

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

#define WORLD cWorld

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    
    InstanceData inst = TreeInstanceBuffer[input.instanceID];
    
    float3x3 worldMat3 = (float3x3) WORLD;
    float3 correctedLocalPos = mul(input.pos, worldMat3) * inst.scale;
    float3 finalWorldPos = correctedLocalPos + inst.position;

    output.worldPos = finalWorldPos;
    output.position = mul(float4(finalWorldPos, 1.0f), VIEW);
    output.position = mul(output.position, PROJ);
    output.texCoord = input.uv;

    output.normal = normalize(mul(input.normal, worldMat3));
    
    output.normal = normalize(mul(input.normal, worldMat3));
    output.tangent = normalize(mul(input.tangent, worldMat3));
    output.binormal = normalize(mul(input.binormal, worldMat3));

    return output;
} // main