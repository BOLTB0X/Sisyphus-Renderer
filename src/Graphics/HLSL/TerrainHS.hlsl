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

// 카메라 거리에 따른 분할 계수(LOD)를 계산하는 함수
float CalcTessFactor(float3 patchCenterPos)
{
    float dist = distance(patchCenterPos, CAMERA_POSITION);
    
    float tess = saturate((MAX_TESS_DISTANCE - dist) / (MAX_TESS_DISTANCE - MIN_TESS_DISTANCE));
    
    return lerp(MIN_TESS_FACTOR, MAX_TESS_FACTOR, tess);
} // CalcTessFactor

// -----------------------------------------------------------------------------
// Patch Constant Function (패치마다 한 번 실행)
// -----------------------------------------------------------------------------
PatchTess CalcHSPatchConstants(
    InputPatch<VS_OUT, 4> patch,
    uint patchID : SV_PrimitiveID)
{
    PatchTess pt;
    
    // 패치의 4개 모서리 중심점을 구하여 대략적인 패치 위치를 계산
    float3 centerPos = 0.25f * (patch[0].posW + patch[1].posW + patch[2].posW + patch[3].posW);
    float tessLevel = CalcTessFactor(centerPos);
    
    // 4개의 엣지(Edge) 분할 계수
    pt.edgeTess[0] = tessLevel;
    pt.edgeTess[1] = tessLevel;
    pt.edgeTess[2] = tessLevel;
    pt.edgeTess[3] = tessLevel;
    
    // 내부(Inside) 분할 계수 (X, Y 방향)
    pt.insideTess[0] = tessLevel;
    pt.insideTess[1] = tessLevel;
    
    return pt;
} // CalcHSPatchConstants

// -----------------------------------------------------------------------------
// Hull Shader (제어점마다 한 번 실행 - 즉, 패치당 4번)
// -----------------------------------------------------------------------------
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
    
    // 제어점 데이터를 그대로 통과시킵니다.
    output.posW = patch[i].posW;
    output.uv = patch[i].uv;
    
    return output;
} // main