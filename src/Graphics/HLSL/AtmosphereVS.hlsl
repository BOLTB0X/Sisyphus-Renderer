// AtmosphereVS.hlsl
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}; // MatrixBuffer

struct VS_INPUT
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD0;
}; // VS_INPUT

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
}; // PS_INPUT


PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    input.position.w = 1.0f;
    
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.position = output.position.xyww;
    output.texCoord = input.position;
    
    return output;
} // main