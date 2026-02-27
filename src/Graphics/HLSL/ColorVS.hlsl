cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}; // MatrixBuffer

struct VS_INPUT
{
    float4 pos   : POSITION;
    float4 color : COLOR;
}; // VS_INPUT

struct PS_INPUT
{
    float4 pos   : SV_POSITION;
    float4 color : COLOR;
}; // PS_INPUT

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    input.pos.w = 1.0f;

    output.pos = mul(input.pos, worldMatrix);
    output.pos = mul(output.pos, viewMatrix);
    output.pos = mul(output.pos, projectionMatrix);
    
    output.color = input.color;
    return output;
} // main