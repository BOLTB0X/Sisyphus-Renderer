// SkinnedVS.hlsl
#include "Common.hlsli"

struct VS_IN
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    uint4  boneIndices : BLENDINDICES;
    float4 boneWeights : BLENDWEIGHT;
}; // VS_IN

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : TEXCOORD;
}; // VS_OUT

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

cbuffer BoneBuffer : register(b3)
{
    matrix cBoneTransforms[256];
}; // BoneBuffer

#define WORLD cWorld

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    float4 skinnedPos =
        mul(float4(input.position, 1.0f), cBoneTransforms[input.boneIndices.x]) * input.boneWeights.x +
        mul(float4(input.position, 1.0f), cBoneTransforms[input.boneIndices.y]) * input.boneWeights.y +
        mul(float4(input.position, 1.0f), cBoneTransforms[input.boneIndices.z]) * input.boneWeights.z +
        mul(float4(input.position, 1.0f), cBoneTransforms[input.boneIndices.w]) * input.boneWeights.w;
    skinnedPos.w = 1.0f;

    float3 skinnedNormal =
        mul(input.normal, (float3x3) cBoneTransforms[input.boneIndices.x]) * input.boneWeights.x +
        mul(input.normal, (float3x3) cBoneTransforms[input.boneIndices.y]) * input.boneWeights.y +
        mul(input.normal, (float3x3) cBoneTransforms[input.boneIndices.z]) * input.boneWeights.z +
        mul(input.normal, (float3x3) cBoneTransforms[input.boneIndices.w]) * input.boneWeights.w;

    float3 skinnedTangent =
        mul(input.tangent, (float3x3) cBoneTransforms[input.boneIndices.x]) * input.boneWeights.x +
        mul(input.tangent, (float3x3) cBoneTransforms[input.boneIndices.y]) * input.boneWeights.y +
        mul(input.tangent, (float3x3) cBoneTransforms[input.boneIndices.z]) * input.boneWeights.z +
        mul(input.tangent, (float3x3) cBoneTransforms[input.boneIndices.w]) * input.boneWeights.w;

    float3 skinnedBinormal =
        mul(input.binormal, (float3x3) cBoneTransforms[input.boneIndices.x]) * input.boneWeights.x +
        mul(input.binormal, (float3x3) cBoneTransforms[input.boneIndices.y]) * input.boneWeights.y +
        mul(input.binormal, (float3x3) cBoneTransforms[input.boneIndices.z]) * input.boneWeights.z +
        mul(input.binormal, (float3x3) cBoneTransforms[input.boneIndices.w]) * input.boneWeights.w;

    float4 worldPos = mul(skinnedPos, WORLD);
    output.worldPos = worldPos.xyz;
    output.pos = mul(worldPos, VIEW);
    output.pos = mul(output.pos, PROJ);

    float3x3 worldRot = (float3x3) WORLD;
    output.normal = normalize(mul(skinnedNormal, worldRot));
    output.tangent = normalize(mul(skinnedTangent, worldRot));
    output.binormal = normalize(mul(skinnedBinormal, worldRot));
    output.uv = input.uv;
    return output;
} // main