// InstancedDepthVS.hlsl
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
    uint  instanceID : SV_InstanceID;
}; // VS_IN

cbuffer MatrixBuffer : register(b0)
{
    matrix cWorld;
    matrix cView;
    matrix cProj;
}; // MatrixBuffer

float4 main(VS_IN input) : SV_POSITION
{
    InstanceData inst = TreeInstanceBuffer[input.instanceID];

    float3 correctedLocalPos = mul(input.pos, (float3x3) cWorld) * inst.scale;
    float3 finalWorldPos = correctedLocalPos + inst.position;

    float4 pos = mul(float4(finalWorldPos, 1.0f), cView);
    pos = mul(pos, cProj);
    return pos;
} // main