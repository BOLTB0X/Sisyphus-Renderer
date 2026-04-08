// Common.hlsli
#ifndef _COMMON_HLSLI_
#define _COMMON_HLSLI_

cbuffer FrameBuffer : register(b0)
{
    matrix cView;
    matrix cProjection;
    matrix cViewInv;
    matrix cProjInv;
    
    float3 cCameraPosition;
    float  fPadding1;
    
    float2 cScreenResolution;
    float  cTime;
    float  fPadding2;
}; // FrameBuffer

cbuffer DirectionalLightBuffer : register(b1)
{
    float3 cLightDirection;
    float  lPadding1;
    
    float4 cLightAmbient;
    float4 cLightDiffuse;
    
    float3 cLightLookAt;
    float  lPadding2;
    
    matrix cLightView;
    matrix cLightProj;
}; // DirectionalLightBuffer

static float depth_to_meter(float z, matrix proj)
{
    float c = proj._33;
    float d = proj._43;

    return d / (z - c);
} // depth_to_meter

static float meter_to_depth(float distanceFeet, matrix proj)
{
    // 전치된 Projection_ 행렬에서 필요한 paraFeets를 추출
    float c = proj._33;
    float d = proj._43;
    float z = c + d / distanceFeet;
    return z;
} // meter_to_depth

static float3 get_world_from_depth(float2 uv, float depth, float4x4 invView, float4x4 invProj)
{
    float4 clipPos;
    clipPos.x = uv.x * 2.0f - 1.0f;
    clipPos.y = (1.0f - uv.y) * 2.0f - 1.0f;
    clipPos.z = depth;
    clipPos.w = 1.0f;

    float4 worldPos = mul(clipPos, invProj);
    worldPos *= (1.0f / worldPos.w);
    //worldPos /= worldPos.w;
    worldPos = mul(worldPos, invView);

    return worldPos.xyz;
} // get_world_from_depth

static float3 get_box_uvw(float3 worldPos, float3 boxPos, float3 boxSize)
{
    float3 boxMin = boxPos - boxSize * 0.5f;
    float3 boxMax = boxPos + boxSize * 0.5f;
    return saturate((worldPos - boxMin) / (boxMax - boxMin));
} // get_box_uvw

#endif // _COMMON_HLSLI_