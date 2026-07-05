// VolumetricFogCS.hlsl
// 터레인 높이맵 기반 하이트 포그 + 볼류메트릭 라이트 스캐터링
#include "Common.hlsli"
#include "Volumetric.hlsli"
#include "ShadowMap.hlsli"

RWTexture2D<float4>    OutTexture : register(u0);
SamplerState           LinearWrapSampler : register(s0);
SamplerState           PointClampSampler : register(s1);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D<float>       SceneDepth : register(t1);
Texture2D              NormalMap : register(t2);
Texture2D<float>       HeightMap : register(t3);
Texture3D              WorleyNoise : register(t4);
Texture2D              TerrainShadowMap : register(t11);

cbuffer VolumetricFogBuffer : register(b2)
{
    // Row 1: 높이 기반 밀도
    float  fBaseHeight;
    float  fHeightFalloff;
    float  fDensity;
    float  fMaxDistance;
    // Row 2: 노이즈 (듬성듬성)
    float  fNoiseScale;
    float  fNoiseStrength;
    float  fWindSpeed;
    float  fPhaseG;
    // Row 3: 색상
    float3 fColor;
    float  fAmbientStrength;
    // Row 4: 경사 감쇠 + 바람 방향
    float  fSlopeMin;
    float  fSlopeMax;
    float2 fWindDirection;
    // Row 5: 지형 월드 매핑
    float  fTerrainWidth;
    float  fTerrainDepth;
    float  fTerrainHeightScale;
    int    fMarchSteps;
    //
    float4 fPadding;
}; // VolumetricFogBuffer

#define FOG_BASE_HEIGHT      fBaseHeight
#define FOG_HEIGHT_FALLOFF   fHeightFalloff
#define FOG_DENSITY          fDensity
#define FOG_MAX_DISTANCE     fMaxDistance
#define FOG_NOISE_SCALE      fNoiseScale
#define FOG_NOISE_STRENGTH   fNoiseStrength
#define FOG_WIND_SPEED       fWindSpeed
#define FOG_PHASE_G          fPhaseG
#define FOG_COLOR            fColor
#define FOG_AMBIENT_STRENGTH fAmbientStrength
#define FOG_SLOPE_MIN        fSlopeMin
#define FOG_SLOPE_MAX        fSlopeMax
#define FOG_WIND_DIRECTION   fWindDirection
#define TERRAIN_WIDTH        fTerrainWidth
#define TERRAIN_DEPTH        fTerrainDepth
#define TERRAIN_HEIGHT_SCALE fTerrainHeightScale
#define FOG_MARCH_STEPS      fMarchSteps

float GetTerrainHeightAt(float3 worldPos)
{
    float2 uv = float2((worldPos.x + TERRAIN_WIDTH * 0.5f) / TERRAIN_WIDTH,
                        (worldPos.z + TERRAIN_DEPTH * 0.5f) / TERRAIN_DEPTH);
    
    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f)
        return 0.0f;

    float h = HeightMap.SampleLevel(LinearWrapSampler, uv, 0).r;
    return h * TERRAIN_HEIGHT_SCALE;
}

float GetFogNoise(float3 worldPos)
{
    float2 windOffset = FOG_WIND_DIRECTION * TIME * FOG_WIND_SPEED;
    float3 samplePos = (worldPos + float3(windOffset.x, 0.0f, windOffset.y)) * FOG_NOISE_SCALE;
    return WorleyNoise.SampleLevel(LinearWrapSampler, samplePos, 0).r;
}

float ComputeFogDensity(float3 worldPos)
{
    float terrainH = GetTerrainHeightAt(worldPos);
    
    float heightAboveGround = (worldPos.y - terrainH) - FOG_BASE_HEIGHT;
    heightAboveGround = max(heightAboveGround, 0.0f);

    float heightDensity = FOG_DENSITY * exp(-FOG_HEIGHT_FALLOFF * heightAboveGround);
    float noise = GetFogNoise(worldPos);
    float patchiness = lerp(1.0f - FOG_NOISE_STRENGTH, 1.0f, noise);
    
    return max(heightDensity * patchiness, 0.0f);
}

float GetFogShadow(float3 worldPos)
{
    float4 lightClipPos = mul(mul(float4(worldPos, 1.0f), LIGHT_VIEW), LIGHT_PROJ);
    return calculate_poisson_shadow(ShadowSampler, TerrainShadowMap, lightClipPos,
        SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);
}

float4 RaymarchFog(float3 ro, float3 rd, float maxDist, uint2 pixelPos)
{
    float3 scatteredLight = float3(0.0f, 0.0f, 0.0f);
    float transmittance = 1.0f;

    if (maxDist <= 0.0f)
        return float4(0.0f, 0.0f, 0.0f, 1.0f);

    float dD = maxDist / (float) FOG_MARCH_STEPS;
    
    // 지터링 값이 0.0 ~ 1.0 이라고 가정
    float d = dD * jittering(float3(pixelPos, TIME));

    float sundotrd = dot(rd, -LIGHT_DIRECTION);
    float phaseFunction = henyey_greenstein(sundotrd, FOG_PHASE_G);
    float3 sunColor = get_dynamic_light_color(LIGHT_DIRECTION).rgb;
    float3 ambientColor = LIGHT_AMBIENT.rgb * FOG_AMBIENT_STRENGTH;
    
    [loop]
    for (int i = 0; i < FOG_MARCH_STEPS; i++)
    {
        float3 p = ro + rd * d;
        float alpha = ComputeFogDensity(p);

        if (alpha > 0.0001f)
        {
            float shadow = GetFogShadow(p);
            float3 lightContrib = sunColor * shadow * phaseFunction + ambientColor;

            float3 S = FOG_COLOR * lightContrib * alpha;
            float dTrans = exp(-alpha * dD);
            float3 Sint = (S - S * dTrans) * (1.0f / max(alpha, 0.0001f));
            
            scatteredLight += transmittance * Sint;
            transmittance *= dTrans;
        }

        d += dD;
        
        if (transmittance <= 0.01f)
            break;
    }

    return float4(scatteredLight, transmittance);
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    OutTexture.GetDimensions(width, height);

    if (DTid.x >= width || DTid.y >= height)
        return;
        
    float2 uv = (float2(DTid.xy) + 0.5f) / float2(width, height);
    float3 ro = CAMERA_POSITION;
    float3 rd = ray_direction_restore(uv, PROJ_INV, VIEW_INV);
    
    float sceneDepth = SceneDepth.SampleLevel(PointClampSampler, uv, 0).r;
    float dist = FOG_MAX_DISTANCE;

    if (sceneDepth > 0.0f && sceneDepth < 1.0f)
    {
        float3 worldPos = get_world_from_depth(uv, sceneDepth, VIEW_INV, PROJ_INV);
        dist = min(length(worldPos - ro), FOG_MAX_DISTANCE);
    }

    float4 col = RaymarchFog(ro, rd, dist, DTid.xy);

    OutTexture[DTid.xy] = col;
}