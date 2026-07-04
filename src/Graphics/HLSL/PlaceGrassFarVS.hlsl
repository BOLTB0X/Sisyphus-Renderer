// PlaceGrassFarVS.hlsl
#include "Common.hlsli"

struct InstanceData
{
    float3 position;
    float  scale;
    float3 normal;
    float  type;
}; // InstanceData

StructuredBuffer<InstanceData> InstanceBuffer : register(t1);

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
    
    float  gDist;
    float  gAlphaCut;
    float2 gPadding;
}; // GrassBuffer

#define GRASS_WIDTH   gGrassWidth
#define GRASS_HEIGHT  gGrassHeight
#define WIND_STRENGTH gWindStrength
#define WIND_SPEED    gWindSpeed
#define LIMIT_DIST    gDist

PS_IN main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    PS_IN output;
    
    InstanceData data = InstanceBuffer[instanceID];
    float dist = length(CAMERA_POSITION - data.position);
    
    if (dist < LIMIT_DIST || dist > LIMIT_DIST * 2.0f)
    {
        output.position = float4(2.0f, 2.0f, 2.0f, 1.0f); // 화면 밖으로 날림
        output.uv = float2(0, 0);
        output.worldPos = float3(0, 0, 0);
        output.dist = 0.0f;
        output.rootWorldPos = float3(0, 0, 0);
        return output;
    }

    // 쿼드 버텍스 위치 및 UV 가져오기
    float2 lp = quad_vertex_pos[vertexID];
    float2 luv = quad_uv[vertexID];

    float3 toCamera = CAMERA_POSITION - data.position;
    toCamera.y = 0.0f;
    float3 camDir = normalize(toCamera);
    float3 right = normalize(cross(float3(0, 1, 0), camDir));
    float3 up = float3(0, 1, 0);
    
    float windX = sin(TIME * WIND_SPEED + data.position.x * 0.3f) * WIND_STRENGTH;
    float windZ = cos(TIME * WIND_SPEED * 0.8f + data.position.z * 0.3f) * WIND_STRENGTH * 0.5f;
    float3 windOffset = float3(windX, 0, windZ) * lp.y;

    float3 worldPos = data.position
        + right * lp.x * GRASS_WIDTH * data.scale
        + up * lp.y * GRASS_HEIGHT * data.scale
        + windOffset;
    
    output.position = mul(mul(float4(worldPos, 1.0f), VIEW), PROJ);
    output.uv = luv;
    output.worldPos = worldPos;
    output.dist = dist;
    output.rootWorldPos = data.position;
    
    return output;
} // main