// QuadTreeGrassFarVS.hlsl
#include "Common.hlsli"

struct VS_IN
{
    float3 worldPos : INSTANCE_POSITION;
    float  scale : INSTANCE_SCALE;
    float2 uv : INSTANCE_UV;
    float2 padding : INSTANCE_PADDING;
}; // VS_IN

struct PS_IN
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float  dist : TEXCOORD2;
    float3 rootWorldPos : TEXCOORD3;
}; // PS_IN

cbuffer GrassBuffer : register(b3)
{
    float gGrassWidth;
    float gGrassHeight;
    float gWindStrength;
    float gWindSpeed;
    
    float gDist;
    float gAlphaCut;
    float2 gPadding;
}; // GrassBuffer

#define GRASS_WIDTH   gGrassWidth
#define GRASS_HEIGHT  gGrassHeight
#define WIND_STRENGTH gWindStrength
#define WIND_SPEED    gWindSpeed
#define LIMIT_DIST    gDist
#define ALPHA_CUT     gAlphaCut

PS_IN main(VS_IN input, uint vertexID : SV_VertexID)
{
    PS_IN output;
    
    float dist = length(CAMERA_POSITION - input.worldPos);
    
    if (dist < LIMIT_DIST || dist > LIMIT_DIST * 2.0f)
    {
        output.position = float4(2.0f, 2.0f, 2.0f, 1.0f);
        output.uv = float2(0, 0);
        output.worldPos = float3(0, 0, 0);
        output.dist = 0.0f;
        return output;
    }

    float2 lp = quad_vertex_pos[vertexID];
    float2 luv = quad_uv[vertexID];

    // Y축 고정 빌보드
    float3 toCamera = CAMERA_POSITION - input.worldPos;
    toCamera.y = 0.0f;
    float3 camDir = normalize(toCamera);
    float3 right = normalize(cross(float3(0, 1, 0), camDir));
    float3 up = float3(0, 1, 0);

    // 바람
    float windX = sin(TIME * WIND_SPEED + input.worldPos.x * 0.3f) * WIND_STRENGTH;
    float windZ = cos(TIME * WIND_SPEED * 0.8f + input.worldPos.z * 0.3f) * WIND_STRENGTH * 0.5f;
    float3 windOffset = float3(windX, 0, windZ) * lp.y; // 위쪽만

    float3 worldPos = input.worldPos
        + right * lp.x * GRASS_WIDTH * input.scale
        + up * lp.y * GRASS_HEIGHT * input.scale
        + windOffset;

    output.position = mul(mul(float4(worldPos, 1.0f), VIEW), PROJ);
    output.uv = luv;
    output.worldPos = worldPos;
    output.dist = length(CAMERA_POSITION - input.worldPos);
    output.rootWorldPos = input.worldPos;
    return output;
} // main