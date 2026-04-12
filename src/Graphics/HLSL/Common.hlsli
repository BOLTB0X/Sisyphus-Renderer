// Common.hlsli
#ifndef _COMMON_HLSLI_
#define _COMMON_HLSLI_

#define KM           1000.0f
#define PI           3.14159265f

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

static float3 ray_direction_restore(float2 uv, matrix projInv, matrix viewInv)
{
    float2 ndc = uv * 2.0f - 1.0f;
    ndc.y = -ndc.y;

    float4 target = mul(float4(ndc.x, ndc.y, 1.0f, 1.0f), projInv);
    target /= target.w; // Perspective Divide

    float3 worldDir = mul(target.xyz, (float3x3) viewInv);
    
    return normalize(worldDir);
} // ray_direction_restore

static float3 get_box_uvw(float3 worldPos, float3 boxPos, float3 boxSize)
{
    float3 boxMin = boxPos - boxSize * 0.5f;
    float3 boxMax = boxPos + boxSize * 0.5f;
    return saturate((worldPos - boxMin) / (boxMax - boxMin));
} // get_box_uvw

static float2 get_spherical_uv(float3 rd)
{
    float2 uv;
    uv.x = atan2(rd.x, rd.z) / (2.0f * PI) + 0.5f;
    uv.y = 0.5f - (asin(clamp(rd.y, -1.0f, 1.0f)) / PI);
    return uv;
} // get_spherical_uv

#endif // _COMMON_HLSLI_