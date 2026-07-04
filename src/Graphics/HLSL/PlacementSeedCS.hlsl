// PlacementSeedCS.hlsl
#include "Noise.hlsli"

cbuffer PlacementBuffer : register(b4)
{
    float3 cameraPos;
    float waterLevel;

    float terrainWidth;
    float terrainDepth;
    float grassDensity;
    float treeDensity;
    
    float placementDist;
    float heightScale;
    float2 pPadding;
    
    float treeBaseOffset;
    float treeScaleMin;
    float treeScaleMax;
    float treeJitterRange;

    float treeBlockSize;
    float seedCutoff;
    float treeCutoff;
    float grassCutoff;
}; // PlacementBuffer

cbuffer WorldBuffer : register(b5)
{
    matrix cWorld;
}; // WorldBuffer

struct InstanceData
{
    float3 position;
    float scale;
    float3 normal;
    float type; // 0: Near, 1: Far, 2: Tree
}; // InstanceData

#define CAMERA_POS     cameraPos
#define WATER_LEVEL    waterLevel
#define TERRAIN_WIDTH  terrainWidth
#define TERRAIN_DEPTH  terrainDepth  
#define GRASS_DENSITY  grassDensity
#define TREE_DENSITY   treeDensity
#define PLACEMENT_DIST placementDist
#define HEIGHT_SCALE   heightScale

#define TREE_BASE_OFFSET  treeBaseOffset
#define TREE_SCALE_MIN    treeScaleMin
#define TREE_SCALE_MAX    treeScaleMax
#define TREE_JITTER_RANGE treeJitterRange
#define TREE_BLOCK_SIZE   treeBlockSize
#define SEED_CUTOFF       seedCutoff
#define TREE_CUTOFF       treeCutoff
#define GRASS_CUTOFF      grassCutoff

AppendStructuredBuffer<InstanceData> NearGrassBuffer : register(u0);
AppendStructuredBuffer<InstanceData> FarGrassBuffer : register(u1);
AppendStructuredBuffer<InstanceData> TreeBuffer : register(u2);

Texture2D<float>  HeightMap : register(t0);
Texture2D<float4> NormalMap : register(t1);
SamplerState      LinearSampler : register(s0);

float GetTerrainHeight(float3 worldPos)
{
    float2 uv = float2((worldPos.x + TERRAIN_WIDTH * 0.5f) / TERRAIN_WIDTH,
                       (worldPos.z + TERRAIN_DEPTH * 0.5f) / TERRAIN_DEPTH);

    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f)
        return -9999.0f;

    float h = HeightMap.SampleLevel(LinearSampler, uv, 0).r;
    return h * HEIGHT_SCALE;
} // GetTerrainHeight

[numthreads(8, 1, 8)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float worldX = (float) DTid.x - (TERRAIN_WIDTH * 0.5f);
    float worldZ = (float) DTid.z - (TERRAIN_DEPTH * 0.5f);

    float2 uv = float2((float) DTid.x / TERRAIN_WIDTH, (float) DTid.z / TERRAIN_DEPTH);
    float height = HeightMap.SampleLevel(LinearSampler, uv, 0);
    float3 normal = NormalMap.SampleLevel(LinearSampler, uv, 0).xyz * 2.0f - 1.0f;
    normal = normalize(normal);
    float actualHeight = height * HEIGHT_SCALE;

    if (actualHeight <= WATER_LEVEL + 1.0f || normal.y < SEED_CUTOFF)
    {
        return;
    }

    float maxDist = PLACEMENT_DIST * 8.0f;
    float dist = distance(CAMERA_POS.xz, float2(worldX, worldZ));
    if (dist > maxDist)
    {
        return;
    }

    bool treePlaced = false;
    float treeRandomVal = hash_uint2(DTid.xz + 999);

    if (treeRandomVal <= TREE_DENSITY && normal.y > TREE_CUTOFF)
    {
        float treeJitterX = (hash_uint2(DTid.xz + 4321) - 0.5f) * TREE_JITTER_RANGE;
        float treeJitterZ = (hash_uint2(DTid.xz + 8765) - 0.5f) * TREE_JITTER_RANGE;
        float3 treeXZ = float3(worldX + treeJitterX, 0, worldZ + treeJitterZ);

        float treeGroundY = GetTerrainHeight(treeXZ);

        if (treeGroundY > WATER_LEVEL + 1.0f && treeGroundY > -9990.0f)
        {
            InstanceData treeData;
            treeData.position = float3(treeXZ.x, treeGroundY + TREE_BASE_OFFSET, treeXZ.z);

            float treeScaleT = hash_uint2(DTid.xz + 777);
            treeData.scale = lerp(TREE_SCALE_MIN, TREE_SCALE_MAX, treeScaleT);
            treeData.normal = normal;
            treeData.type = 2.0f;

            TreeBuffer.Append(treeData);
            treePlaced = true;
        }
    }

    if (treePlaced)
    {
        return;
    }
    
    float grassMaxDist = PLACEMENT_DIST * 4.0f;
    if (dist > grassMaxDist)
    {
        return;
    }

    float falloff = saturate(dist / maxDist);
    float currentDensity = lerp(GRASS_DENSITY, GRASS_DENSITY * 0.1f, falloff);

    [unroll(2)]
    for (int i = 0; i < 2; ++i)
    {
        float randomVal = hash_uint2(DTid.xz + i * 123);
        if (randomVal > currentDensity)
            continue;

        float jitterX = (hash_uint2(DTid.xz + i + 13) - 0.5f) * 1.5f;
        float jitterZ = (hash_uint2(DTid.xz + i + 73) - 0.5f) * 1.5f;
        float3 pos = float3(worldX + jitterX, 0, worldZ + jitterZ);

        float terrainY = GetTerrainHeight(pos);
        if (terrainY <= WATER_LEVEL + 0.5f || normal.y < GRASS_CUTOFF || terrainY < -9990.0f)
        {
            continue;
        }

        InstanceData data;
        data.position = float3(pos.x, terrainY + 0.05f, pos.z);
        data.scale = lerp(0.8f, 1.3f, hash_uint2(DTid.xz + i + 99));
        data.normal = normal;

        float dist3D = length(CAMERA_POS - data.position);
        if (dist3D < PLACEMENT_DIST)
        {
            data.type = 0.0f;
            NearGrassBuffer.Append(data);
        }
        else if (dist3D < maxDist)
        {
            data.type = 1.0f;
            FarGrassBuffer.Append(data);
        }
    }
} // main