// StoneVS.hlsl
#include "Common.hlsli"

//cbuffer CameraBuffer : register(b0) {
//    matrix worldMatrix;
//    matrix viewMatrix;
//    matrix projectionMatrix;
//    float3 cameraPosition;
//    float pad;
//}; // CameraBuffer

struct VS_INPUT {
    float4 position : POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    float3 binormal : BINORMAL;
}; // VS_INPUT

struct PS_INPUT {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal   : NORMAL;
    float3 worldPos : TEXCOORD1;
}; // PS_INPUT

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    input.position.w = 1.0f;
    
    output.position = mul(input.position, cWorld);
    //output.position = mul(input.position, worldMatrix);
    output.worldPos = output.position.xyz;
    output.position = mul(output.position, cView);
    output.position = mul(output.position, cProjection);
    
    output.texCoord = input.texCoord;
    output.normal = mul(input.normal, (float3x3) cWorld);
    
    return output;
} // main