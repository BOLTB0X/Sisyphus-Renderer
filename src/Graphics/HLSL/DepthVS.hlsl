// DepthVS.hlsl
cbuffer MatrixBuffer : register(b0)
{
    matrix mWorldMatrix;
    matrix mViewMatrix;
    matrix mProjectionMatrix;
}; // MatrixBuffer

struct VS_IN
{
    float4 position : POSITION;
}; // VS_IN

struct PS_IN
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXTURE0;
}; // PS_IN

PS_IN main(VS_IN input)
{
    PS_IN output;
    
    input.position.w = 1.0f;

    output.position = mul(input.position, mWorldMatrix);
    output.position = mul(output.position, mViewMatrix);
    output.position = mul(output.position, mProjectionMatrix);
    output.depthPosition = output.position;
	
    return output;
} // main