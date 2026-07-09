// DeferredFogPS.hlsl
#include "Common.hlsli"
#include "Volumetric.hlsli"
#include "PostProcess.hlsli"

SamplerState LinearWrapSampler : register(s0);
SamplerState PointClampSampler : register(s1);

Texture2D SceneTex : register(t0);
Texture2D DepthTex : register(t1);
Texture3D WorleyNoise : register(t4);
Texture2D NormalMap : register(t5);

cbuffer DeferredFogBuffer : register(b2)
{
    // Row 1: 거리 기반
    float fStartDistance;
    float fDensity;
    float fMaxDistance;
    float fBaseHeight;
    // Row 2: 높이 기반
    float fHeightFalloff;
    float fNoiseScale;
    float fNoiseStrength;
    float fWindSpeed;
    // Row 3: 색상 + 바람
    float3 fColor;
    float fWindDirPad; // 사용 안 함, 정렬용
    // Row 4
    float2 fWindDirection;
    float fSlopeMin;
    float fSlopeMax;
} // DeferredFogBuffer

#define FOG_START_DISTANCE fStartDistance
#define FOG_DENSITY        fDensity
#define FOG_MAX_DISTANCE   fMaxDistance
#define FOG_BASE_HEIGHT    fBaseHeight
#define FOG_HEIGHT_FALLOFF fHeightFalloff
#define FOG_NOISE_SCALE    fNoiseScale
#define FOG_NOISE_STRENGTH fNoiseStrength
#define FOG_WIND_SPEED     fWindSpeed
#define FOG_COLOR          fColor
#define FOG_WIND_DIRECTION fWindDirection
#define FOG_SLOPE_MIN      fSlopeMin
#define FOG_SLOPE_MAX      fSlopeMax

float GetFogNoise(float3 worldPos)
{
    float2 windOffset = FOG_WIND_DIRECTION * TIME * FOG_WIND_SPEED;
    float3 samplePos = (worldPos + float3(windOffset.x, 0.0f, windOffset.y)) * FOG_NOISE_SCALE;
    return WorleyNoise.SampleLevel(LinearWrapSampler, samplePos, 0).r;
} // GetFogNoise

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 scene = SceneTex.Sample(PointClampSampler, input.uv);
    float sceneDepth = DepthTex.Sample(PointClampSampler, input.uv).r;

    if (sceneDepth >= 1.0f)
    {
        return scene;
    }

    float3 worldPos = get_world_from_depth(input.uv, sceneDepth, VIEW_INV, PROJ_INV);
    float dist = length(worldPos - CAMERA_POSITION);

    float pixelDistance = max(dist - FOG_START_DISTANCE, 0.0f);
    float distanceFactor = 1.0f - exp(-FOG_DENSITY * pixelDistance);
    distanceFactor *= saturate(1.0f - (dist - FOG_MAX_DISTANCE) / max(FOG_MAX_DISTANCE * 0.2f, 0.001f));

    float heightAboveBase = max(worldPos.y - FOG_BASE_HEIGHT, 0.0f);
    float heightFactor = exp(-heightAboveBase * FOG_HEIGHT_FALLOFF);

    float noise = GetFogNoise(worldPos);
    float noiseFactor = lerp(1.0f - FOG_NOISE_STRENGTH, 1.0f, noise);
    float3 normal = NormalMap.Sample(PointClampSampler, input.uv).xyz * 2.0f - 1.0f;
    normal = normalize(normal);
    float slopeFactor = smoothstep(FOG_SLOPE_MIN, FOG_SLOPE_MAX, normal.y);

    float fogFactor = saturate(distanceFactor * heightFactor * noiseFactor * slopeFactor);

    float3 finalColor = lerp(scene.rgb, FOG_COLOR, fogFactor);

    return float4(finalColor, scene.a);
} // main