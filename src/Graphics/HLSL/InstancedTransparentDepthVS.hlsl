// InstancedTransparentDepthVS.hlsl
struct InstanceData
{
    float3 position;
    float scale;
    float3 normal;
    float type;
}; // InstanceData

StructuredBuffer<InstanceData> TreeInstanceBuffer : register(t5);

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    uint instanceID : SV_InstanceID;
}; // VS_IN

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
}; // VS_OUT

cbuffer MatrixBuffer : register(b0)
{
    matrix cWorld;
    matrix cView;
    matrix cProj;
}; // MatrixBuffer

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    InstanceData inst = TreeInstanceBuffer[input.instanceID];

    float3 correctedLocalPos = mul(input.pos, (float3x3) cWorld) * inst.scale;
    float3 finalWorldPos = correctedLocalPos + inst.position;

    output.pos = mul(float4(finalWorldPos, 1.0f), cView);
    output.pos = mul(output.pos, cProj);
    output.uv = input.uv;

    return output;
} // main