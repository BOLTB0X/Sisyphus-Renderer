// TerrainHS.hlsl
#include "Common.hlsli"

struct VS_OUT
{
    float3 posW : POSITION;
    float2 uv : TEXCOORD;
}; // VS_OUT

// 제어점 당 1개씩 출력
struct HS_OUT
{
    float3 posW : POSITION;
    float2 uv : TEXCOORD;
}; // HS_OUT

// 패치 상수 데이터
struct PatchTess
{
    float edgeTess[4] : SV_TessFactor;
    float insideTess[2] : SV_InsideTessFactor;
}; // PatchTess

cbuffer TessellationControlBuffer : register(b3)
{
    float  tMinTessDist;
    float  tMaxTessDist;
    float  tMinTessFactor;
    float  tMaxTessFactor;
    
    float4 tPadding;
}; // TessellationControlBuffer

#define MIN_TESS_DISTANCE tMinTessDist
#define MAX_TESS_DISTANCE tMaxTessDist
#define MIN_TESS_FACTOR   tMinTessFactor
#define MAX_TESS_FACTOR   tMaxTessFactor

float CalcTessFactor(float3 patchCenterPos)
{
    float dist = distance(patchCenterPos, CAMERA_POSITION);
    
    float tess = saturate((MAX_TESS_DISTANCE - dist) / (MAX_TESS_DISTANCE - MIN_TESS_DISTANCE));
    
    return lerp(MIN_TESS_FACTOR, MAX_TESS_FACTOR, tess);
} // CalcTessFactor

PatchTess CalcHSPatchConstants(
    InputPatch<VS_OUT, 4> patch,
    uint patchID : SV_PrimitiveID)
{
    PatchTess pt;

    float3 edgeMid0 = 0.5f * (patch[0].posW + patch[3].posW); // edge0: 0-3
    float3 edgeMid1 = 0.5f * (patch[0].posW + patch[1].posW); // edge1: 0-1
    float3 edgeMid2 = 0.5f * (patch[1].posW + patch[2].posW); // edge2: 1-2
    float3 edgeMid3 = 0.5f * (patch[2].posW + patch[3].posW); // edge3: 2-3

    pt.edgeTess[0] = CalcTessFactor(edgeMid0);
    pt.edgeTess[1] = CalcTessFactor(edgeMid1);
    pt.edgeTess[2] = CalcTessFactor(edgeMid2);
    pt.edgeTess[3] = CalcTessFactor(edgeMid3);

    float3 centerPos = 0.25f * (patch[0].posW + patch[1].posW + patch[2].posW + patch[3].posW);
    float insideTess = CalcTessFactor(centerPos);
    pt.insideTess[0] = insideTess;
    pt.insideTess[1] = insideTess;

    return pt;
} // CalcHSPatchConstants

[domain("quad")] // 4각형 패치 사용
[partitioning("fractional_even")] // 부드러운 LOD 전환을 위해 소수점 분할 사용
[outputtopology("triangle_ccw")] // 최종 출력은 시계방향 삼각형
[outputcontrolpoints(4)] // 출력할 제어점 개수
[patchconstantfunc("CalcHSPatchConstants")] // 위에서 정의한 함수 지정
HS_OUT main(
    InputPatch<VS_OUT, 4> patch,
    uint i : SV_OutputControlPointID,
    uint patchId : SV_PrimitiveID)
{
    HS_OUT output;
    
    output.posW = patch[i].posW;
    output.uv = patch[i].uv;
    
    return output;
} // main