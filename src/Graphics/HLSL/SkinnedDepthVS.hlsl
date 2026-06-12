// SkinnedDepthVS.hlsl
cbuffer MatrixBuffer : register(b0)
{
    matrix mWorldMatrix;
    matrix mViewMatrix;
    matrix mProjectionMatrix;
}; // MatrixBuffer

cbuffer BoneBuffer : register(b3)
{
    matrix gBoneMatrices[256];
}; // BoneBuffer

struct VS_IN
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    uint4  boneIDs : BLENDINDICES;
    float4 boneWeights : BLENDWEIGHT;
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

    float4 skinnedPos =
        mul(float4(input.position, 1.0f), gBoneMatrices[input.boneIDs.x]) * input.boneWeights.x +
        mul(float4(input.position, 1.0f), gBoneMatrices[input.boneIDs.y]) * input.boneWeights.y +
        mul(float4(input.position, 1.0f), gBoneMatrices[input.boneIDs.z]) * input.boneWeights.z +
        mul(float4(input.position, 1.0f), gBoneMatrices[input.boneIDs.w]) * input.boneWeights.w;
    skinnedPos.w = 1.0f;

    float4 worldPos = mul(skinnedPos, WORLD);
    float4 viewPos = mul(worldPos, VIEW);
    float4 clipPos = mul(viewPos, PROJ);

    output.position = clipPos;
    output.depthPosition = clipPos;
    output.tex = input.tex;

    return output;
} // main