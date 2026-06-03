// TransparentDepthVS.hlsl
// https://www.rastertek.com/dx11win10tut45.html
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
    float2 tex : TEXCOORD0;
}; // VS_IN

struct PS_IN
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXTURE0;
    float2 tex : TEXCOORD1;
}; // PS_IN

#define WORLD mWorldMatrix
#define VIEW  mViewMatrix
#define PROJ  mProjectionMatrix

PS_IN main(VS_IN input)
{
    PS_IN output;
        
    float4 worldPos = mul(float4(input.position, 1.0f), WORLD);
    float4 viewPos = mul(worldPos, VIEW);
    float4 clipPos = mul(viewPos, PROJ);
    
    output.position = clipPos;
    output.depthPosition = clipPos;
    output.tex = input.tex;

    return output;
} // main