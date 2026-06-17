// RefractionVS.hlsl
// https://www.rastertek.com/dx11win10tut31.html
#include "Common.hlsli"

struct VS_IN
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
}; // VS_IN

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float  clip : SV_ClipDistance0;
}; // VS_OUT

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

cbuffer ClipPlaneBuffer : register(b5)
{
    float4 clipPlane;
}; // ClipPlaneBuffer

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    input.position.w = 1.0f;

    output.position = mul(input.position, cWorld);
    output.position = mul(output.position, VIEW);
    output.position = mul(output.position, PROJ);
    
    output.tex = input.tex;
    output.normal = mul(input.normal, (float3x3) cWorld);
    output.normal = normalize(output.normal);

    // Clipping 로직
    // 월드 공간의 정점 위치와 클립 평면을 내적하여 거리를 계산
    output.clip = dot(mul(input.position, cWorld), clipPlane);

    return output;
} // main