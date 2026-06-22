// WaterVS.hlsl
#include "Common.hlsli"

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
}; // VS_IN

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float2 uv : TEXCOORD0;
    float4 reflectPosition : TEXCOORD1;
}; // VS_OUT

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

cbuffer ReflectionMatrixBuffer : register(b4)
{
    matrix reflectionView;
}; // ReflectionMatrixBuffer

#define WORLD           cWorld
#define REFLECTION_VIEW reflectionView

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    float4 worldPos = mul(float4(input.pos, 1.0f), WORLD);
    output.worldPos = worldPos.xyz;
    
    output.pos = mul(worldPos, VIEW);
    output.pos = mul(output.pos, PROJ);
    
    output.uv = input.uv * 10.0f;
    
    output.reflectPosition = mul(worldPos, REFLECTION_VIEW);
    output.reflectPosition = mul(output.reflectPosition, PROJ);
    
    return output;
} // main