// DepthVS.hlsl
cbuffer MatrixBuffer : register(b0)
{
    matrix mWorldMatrix;
    matrix mViewMatrix;
    matrix mProjectionMatrix;
}; // MatrixBuffer

struct VS_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
}; // VS_IN

struct PS_IN
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXTURE0;
}; // PS_IN

#define WORLD mWorldMatrix
#define VIEW  mViewMatrix
#define PROJ  mProjectionMatrix

PS_IN main(VS_IN input)
{
    PS_IN output;
    
    float3 offsetPos = input.position + input.normal * 0.1f;
    
    float4 worldPos = mul(float4(offsetPos, 1.0f), WORLD);
    float4 viewPos = mul(worldPos, VIEW);
    float4 clipPos = mul(viewPos, PROJ);
    
    output.position = clipPos;
    output.depthPosition = clipPos;
	
    return output;
} // main