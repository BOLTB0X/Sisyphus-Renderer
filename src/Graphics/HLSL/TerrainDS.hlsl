// TerrainDS.hlsl
#include "Common.hlsli"

struct PatchTess
{
    float edgeTess[4] : SV_TessFactor;
    float insideTess[2] : SV_InsideTessFactor;
}; // PatchTess

struct HS_OUT
{
    float3 posW : POSITION;
    float2 uv : TEXCOORD;
}; // HS_OUT

struct DS_OUT
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float  height : TEXCOORD1;
}; // DS_OUT

Texture2D    HeightMap : register(t0);
SamplerState LinearSampler : register(s0);

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

cbuffer HeightScaleBuffer : register(b4)
{
    float  hHeightScale;
    float3 hPadding;
}; // HeightScaleBuffer

#define HEIGHT_SCALE hHeightScale


float3 CalculateNormalFromHeightMap(float2 uv, float texelSize)
{
    // 상하좌우 높이 샘플링
    float hL = HeightMap.SampleLevel(LinearSampler, uv + float2(-texelSize, 0.0f), 0).r * HEIGHT_SCALE;
    float hR = HeightMap.SampleLevel(LinearSampler, uv + float2(texelSize, 0.0f), 0).r * HEIGHT_SCALE;
    float hB = HeightMap.SampleLevel(LinearSampler, uv + float2(0.0f, -texelSize), 0).r * HEIGHT_SCALE;
    float hT = HeightMap.SampleLevel(LinearSampler, uv + float2(0.0f, texelSize), 0).r * HEIGHT_SCALE;

    // x, z 축 방향의 접선(Tangent) 벡터 생성 후 외적
    float3 tangentX = normalize(float3(2.0f, hR - hL, 0.0f));
    float3 tangentZ = normalize(float3(0.0f, hT - hB, 2.0f));
    
    return normalize(cross(tangentZ, tangentX));
} // CalculateNormalFromHeightMap

[domain("quad")]
DS_OUT main(
    PatchTess patchTess,
    float2 domain : SV_DomainLocation, // 패치 내부의 0.0 ~ 1.0 사이 분할 좌표 (UV 역할)
    const OutputPatch<HS_OUT, 4> patch)
{
    DS_OUT output;

    // 위치 보간
    float3 bottomPos = lerp(patch[0].posW, patch[1].posW, domain.x);
    float3 topPos = lerp(patch[3].posW, patch[2].posW, domain.x);
    float3 finalPosL = lerp(bottomPos, topPos, domain.y); // 로컬 좌표

    float2 bottomUV = lerp(patch[0].uv, patch[1].uv, domain.x);
    float2 topUV = lerp(patch[3].uv, patch[2].uv, domain.x);
    float2 finalUV = lerp(bottomUV, topUV, domain.y);

    // 높이 적용 및 월드 변환
    float heightOutput = HeightMap.SampleLevel(LinearSampler, finalUV, 0).r;
    finalPosL.y += heightOutput * HEIGHT_SCALE;

    // 로컬 -> 월드 공간 변환
    float3 finalPosW = mul(float4(finalPosL, 1.0f), cWorld).xyz;

    // 노말 계산 및 공간 변환
    float texelSize = 1.0f / 1024.0f;
    output.normal = CalculateNormalFromHeightMap(finalUV, texelSize);
    
    // 최종 투영 변환
    float4 posW = float4(finalPosW, 1.0f);
    output.posH = mul(posW, VIEW);
    output.posH = mul(output.posH, PROJ);
    
    output.posW = finalPosW;
    output.uv = finalUV;
    output.height = finalPosL.y;
    
    return output;
} // main