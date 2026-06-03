// Common.hlsli
#ifndef _COMMON_HLSLI_
#define _COMMON_HLSLI_

#define KM              1000.0f
#define PI              3.14159265f
#define MIN_DIST        float(1e-4f)
#define MAX_DIST        float(1e7f)
#define DEFAULT_AMBIENT float4(0.03f, 0.03f, 0.03f, 1.0f)

cbuffer FrameBuffer : register(b0)
{
    matrix cView;
    matrix cProjection;
    matrix cViewInv;
    matrix cProjInv;
    
    float3 cCameraPosition;
    float  cCameraFov;
    
    float2 cScreenResolution;
    float  cTime;
    float  fPadding2;
}; // FrameBuffer

cbuffer DirectionalLightBuffer : register(b1)
{
    float3 cLightDirection;
    float  dPadding1;
    
    float4 cLightAmbient;
    float4 cLightDiffuse;
    float4 cSunSetLight;
    float4 cNightLight;
    
    float3 cLightLookAt;
    float  dPadding2;
    
    matrix cLightView;
    matrix cLightProj;
    
    matrix cObjectView;
    matrix cObjectProj;
    
    float  cMapWidth;
    float  cMapHeight;
    float  cBias;
    float  cSpread;
    float4 cPadding3;
}; // DirectionalLightBuffer

#define CAMERA_POSITION   cCameraPosition
#define CAMERA_FOV        cCameraFov
#define SCREEN_RESOLUTION cScreenResolution
#define TIME              cTime

#define VIEW         cView
#define PROJ         cProjection
#define VIEW_INV     cViewInv
#define PROJ_INV     cProjInv

#define LIGHT_DIRECTION   normalize(cLightDirection)
#define LIGHT_COLOR       cLightDiffuse
#define LIGHT_AMBIENT     cLightAmbient
#define LIGHT_SUNSET      cSunSetLight
#define LIGHT_NIGHT       cNightLight
#define LIGHT_LOOKAT      cLightLookAt
#define LIGHT_VIEW        cLightView
#define LIGHT_PROJ        cLightProj
#define LIGHT_OBJECT_VIEW cObjectView
#define LIGHT_OBJECT_PROJ cObjectProj

#define SHADOW_MAP_SIZE     float2(cMapWidth, cMapHeight)
#define SHADOW_BIAS         cBias
#define SHADOW_SPREAD       cSpread

// 쿼드 로컬 버텍스 (인덱스 없이 4개)
static const float2 quad_vertex_pos[4] =
{
    float2(-1, 0), // left down
    float2(1, 0), // right down
    float2(-1, 1), // left up
    float2(1, 1) // right up
}; // quad_vertex_pos

static const float2 quad_uv[4] =
{
    float2(0, 1), float2(1, 1),
    float2(0, 0), float2(1, 0)
}; // quad_uv

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

static float4 get_dynamic_light_color(float3 lightDir)
{
    float3 dayColor = LIGHT_COLOR.rgb;
    float3 sunsetColor = LIGHT_SUNSET.rgb;
    float3 nightColor = LIGHT_NIGHT.rgb;

    float t_sunset = smoothstep(-0.4f, 0.0f, lightDir.y);
    float t_night = smoothstep(0.0f, 0.2f, lightDir.y);

    float3 finalColor = lerp(dayColor, sunsetColor, t_sunset);
    
    finalColor = lerp(finalColor, nightColor, t_night);

    return float4(finalColor, 1.0f);
} // get_dynamic_light_color


#endif // _COMMON_HLSLI_