// PlacementCS.hlsl
#include "Noise.hlsli"

cbuffer PlacementBuffer : register(b4)
{
    float3 cameraPos;
    float  waterLevel;

    float  terrainWidth;
    float  terrainDepth;
    float  grassDensity;
    float  treeDensity;
    
    float   placementDist;
    float   heightScale;
    float2  pPadding;
}; // PlacementBuffer

cbuffer WorldBuffer : register(b5)
{
    matrix cWorld;
}; // WorldBuffer

struct InstanceData
{
    float3 position;
    float  scale;
    float3 normal;
    float  type; // 0: Near, 1: Far, 2: Tree
}; // InstanceData

AppendStructuredBuffer<InstanceData> NearGrassBuffer : register(u0);
AppendStructuredBuffer<InstanceData> FarGrassBuffer : register(u1);

Texture2D<float>  HeightMap : register(t0);
Texture2D<float4> NormalMap : register(t1);
SamplerState      LinearSampler : register(s0);

float GetTerrainHeight(float3 worldPos)
{
    float2 uv = float2((worldPos.x + terrainWidth * 0.5f) / terrainWidth,
                       (worldPos.z + terrainDepth * 0.5f) / terrainDepth);

    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f)
        return -9999.0f;

    float h = HeightMap.SampleLevel(LinearSampler, uv, 0).r;
    return h * heightScale;
} // GetTerrainHeight

[numthreads(8, 1, 8)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float worldX = (float) DTid.x - (terrainWidth * 0.5f);
    float worldZ = (float) DTid.z - (terrainDepth * 0.5f);
    
    // 이 위치에서의 노말 샘플링을 위해 UV 계산
    float2 uv = float2((float) DTid.x / terrainWidth, (float) DTid.z / terrainDepth);
    float3 normal = NormalMap.SampleLevel(LinearSampler, uv, 0).xyz * 2.0f - 1.0f;
    normal = normalize(normal);

    [unroll(2)]
    for (int i = 0; i < 2; ++i)
    {
        float randomVal = hash_uint2(DTid.xz + i * 123);
        if (randomVal > grassDensity)
            continue;

        // 지터 적용된 월드 위치
        float jitterX = (hash_uint2(DTid.xz + i + 13) - 0.5f) * 1.5f;
        float jitterZ = (hash_uint2(DTid.xz + i + 73) - 0.5f) * 1.5f;
        float3 pos = float3(worldX + jitterX, 0, worldZ + jitterZ);

        float terrainY = GetTerrainHeight(pos);
        
        // 물 높이 및 유효성 체크
        if (terrainY <= waterLevel + 0.5f || normal.y < 0.3f || terrainY < -9990.0f)
        {
            continue;
        }

        InstanceData data;
        data.position = float3(pos.x, terrainY + 0.05f, pos.z); // 지형 위에 살짝 올림
        data.scale = lerp(0.8f, 1.3f, hash_uint2(DTid.xz + i + 99));
        data.normal = normal;

        float dist = length(cameraPos - data.position);
        if (dist < placementDist)
        {
            data.type = 0.0f;
            NearGrassBuffer.Append(data);
        }
        else if (dist < placementDist * 2.0f)
        {
            data.type = 1.0f;
            FarGrassBuffer.Append(data);
        }
    }
} // main