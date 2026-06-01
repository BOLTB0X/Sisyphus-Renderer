// GrassGS.hlsl
// https://m.blog.naver.com/PostView.naver?blogId=fah204&logNo=221518704599&referrerCode=0&searchKeyword=grass
// https://www.rastertek.com/tertut19.html
// https://www.shadertoy.com/view/lslGR8
#include "Common.hlsli"
#include "Maths.hlsli"

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

static const float angles[3] = { 0.0f, 1.0472f, 2.0944f };

struct GS_IN
{
    float3 worldPos : POSITION;
    float  height : HEIGHT;
    float2 uv : TEXCOORD;
}; // GS_IN

struct GS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
    float3 rootWorldPos : TEXCOORD2;
}; // GS_OUT

#define GRASS_WIDTH   gGrassWidth
#define GRASS_HEIGHT  gGrassHeight
#define WIND_STRENGTH gWindStrength
#define WIND_SPEED    gWindSpeed
#define LIMIT_DIST    gDist
#define ALPHA_CUT     gAlphaCut

[maxvertexcount(12)]
void main(point GS_IN input[1], inout TriangleStream<GS_OUT> stream)
{
    float3 root = input[0].worldPos;
    float dist = length(root - CAMERA_POSITION);

    if (dist > LIMIT_DIST)
    {
        return;
    }
    
    float height = input[0].height * GRASS_HEIGHT;

    float randRot = get_random_rotation_plane(root.xz);

    float windX = sin(TIME * WIND_SPEED + root.x * 0.3f + root.z * 0.2f) * WIND_STRENGTH;
    float windZ = cos(TIME * WIND_SPEED * 0.8f + root.z * 0.3f) * WIND_STRENGTH * 0.5f;
   
    
    int billboardCount = 3;
    if (dist > LIMIT_DIST / 2)
        billboardCount = 1;
    else if (dist > LIMIT_DIST / 4)
        billboardCount = 2;

    [unroll]
    for (int d = 0; d < billboardCount; ++d)
    {
        float3x3 rot = rotate_Y(angles[d] + randRot);
        float3 right = mul(float3(GRASS_WIDTH, 0, 0), rot);

        float3 v[4] =
        {
            root - right,
            root + right,
            root - right + float3(windX, height, windZ),
            root + right + float3(windX, height, windZ)
        };

        float2 uvs[4] =
        {
            float2(0, 1), float2(1, 1),
            float2(0, 0), float2(1, 0)
        };

        float3 normal = normalize(cross(right, float3(0, 1, 0)));
        int stripIndices[4] = { 0, 2, 1, 3 };
        
        GS_OUT p;
        
        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            p.worldPos = v[stripIndices[i]];
            p.rootWorldPos = root;
            p.position = mul(mul(float4(p.worldPos, 1.0f), VIEW), PROJ);
            p.uv = uvs[stripIndices[i]];
            p.normal = normal;
            stream.Append(p);
        } // for (int i = 0; i < 4; ++i)
        
        stream.RestartStrip();
    } // for (int d = 0; d < 3; ++d)
} // main