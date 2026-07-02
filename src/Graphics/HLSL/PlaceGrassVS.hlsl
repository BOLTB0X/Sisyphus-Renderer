// PlaceGrassVS.hlsl
#include "Common.hlsli"

struct InstanceData
{
    float3 position;
    float  scale;
    float3 normal;
    float  type;
}; // InstanceData

StructuredBuffer<InstanceData> InstanceBuffer : register(t1);

struct VS_OUT
{
    float3 worldPos : POSITION;
    float  height : HEIGHT;
    float2 uv : TEXCOORD;
}; // VS_OUT

VS_OUT main(uint instanceID : SV_InstanceID)
{
    VS_OUT output;
    
    InstanceData data = InstanceBuffer[instanceID];

    output.worldPos = data.position;
    output.height = data.scale;
    
    output.uv = float2(0.0f, 0.0f);

    return output;
} // main