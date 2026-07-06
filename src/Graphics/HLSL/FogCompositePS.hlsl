// FogCompositePS.hlsl
#include "Common.hlsli"
#include "Volumetric.hlsli"
#include "ShadowMap.hlsli"
#include "PostProcess.hlsli"

SamplerState           LinearWrapSampler : register(s0);
SamplerState           PointClampSampler : register(s1);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D        SceneTex : register(t0);
Texture2D        DepthTex : register(t1);
Texture2D<float> NoiseMap : register(t2);
Texture2D<float> HeightMap : register(t3);
Texture3D        WorleyNoise : register(t4);
Texture2D        NormalMap : register(t5);
Texture2D        TerrainShadowMap : register(t11);

cbuffer VolumetricFogBuffer : register(b2)
{
    float  fBaseHeight;
    float  fHeightFalloff;
    float  fDensity;
    float  fMaxDistance;
    
    float  fNoiseScale;
    float  fNoiseStrength;
    float  fWindSpeed;
    float  vPadding1;
    
    float3 fColor;
    float  fAmbientStrength;
    
    float2 fWindDirection;
    float2 vPadding2;
    
    float fTerrainWidth;
    float fTerrainDepth;
    float fTerrainHeightScale;
    int   fMarchSteps;
    
    float4 vPadding3;
} // VolumetricFogBuffer

#define FOG_BASE_HEIGHT      fBaseHeight
#define FOG_HEIGHT_FALLOFF   fHeightFalloff
#define FOG_DENSITY          fDensity
#define FOG_MAX_DISTANCE     fMaxDistance
#define FOG_NOISE_SCALE      fNoiseScale
#define FOG_NOISE_STRENGTH   fNoiseStrength
#define FOG_WIND_SPEED       fWindSpeed
#define FOG_COLOR            fColor
#define FOG_AMBIENT_STRENGTH fAmbientStrength
#define FOG_WIND_DIRECTION   fWindDirection
#define TERRAIN_WIDTH        fTerrainWidth
#define TERRAIN_DEPTH        fTerrainDepth
#define TERRAIN_HEIGHT_SCALE fTerrainHeightScale
#define FOG_MARCH_STEPS      fMarchSteps

float GetTerrainHeightAt(float3 worldPos)
{
    float2 uv = float2((worldPos.x + TERRAIN_WIDTH * 0.5f) / TERRAIN_WIDTH,
                       (worldPos.z + TERRAIN_DEPTH * 0.5f) / TERRAIN_DEPTH);
    
    uv = saturate(uv);

    float h = HeightMap.SampleLevel(LinearWrapSampler, uv, 0).r;
    return h * TERRAIN_HEIGHT_SCALE;
} // GetTerrainHeightAt

float ComputeFogDensity(float3 p, float maxFogHeight)
{
    float terrainHeight = GetTerrainHeightAt(p);
    
    float blendedHeight = lerp(0.0f, terrainHeight, 0.5f);
    float heightAboveGround = p.y - (blendedHeight + FOG_BASE_HEIGHT);
    
    float heightFalloff = exp(-max(heightAboveGround, 0.0f) * FOG_HEIGHT_FALLOFF);

    // 노이즈 계산
    float2 noiseCoord = p.xz * (FOG_NOISE_SCALE * 0.1f);
    noiseCoord += FOG_WIND_DIRECTION * TIME * FOG_WIND_SPEED;
    float baseNoise = NoiseMap.SampleLevel(LinearWrapSampler, noiseCoord, 0).r;

    float3 worleyCoord = p * FOG_NOISE_SCALE;
    float detailNoise = WorleyNoise.SampleLevel(LinearWrapSampler, worleyCoord, 0).r;
    
    float combinedNoise = baseNoise * detailNoise;
    float noiseFactor = lerp(1.0f, combinedNoise, fNoiseStrength);
    noiseFactor = max(noiseFactor, 0.1f);
    
    return FOG_DENSITY * noiseFactor * heightFalloff;
} // ComputeFogDensity

float GetFogShadow(float3 worldPos)
{
    float4 lightClipPos = mul(mul(float4(worldPos, 1.0f), LIGHT_VIEW), LIGHT_PROJ);
    return calculate_poisson_shadow(ShadowSampler, TerrainShadowMap, lightClipPos,
        SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);
} // GetFogShadow

float4 RaymarchFog(float3 ro, float3 rd, float maxDist, uint2 pixelPos)
{
    float maxTerrainFogOffset = TERRAIN_HEIGHT_SCALE * 1.0f;
    float maxFogHeight = FOG_BASE_HEIGHT + maxTerrainFogOffset + (7.0f / max(FOG_HEIGHT_FALLOFF, 0.001f));
    
    float tMin = 0.0f;
    float tMax = maxDist;

    if (ro.y > maxFogHeight && rd.y < -0.001f)
    {
        tMin = max(tMin, (maxFogHeight - ro.y) / rd.y);
    }
    if (ro.y < maxFogHeight && rd.y > 0.001f)
    {
        tMax = min(tMax, (maxFogHeight - ro.y) / rd.y);
    }

    if (tMin >= tMax || tMin > maxDist) 
        return float4(0.0f, 0.0f, 0.0f, 1.0f);

    float segmentLength = tMax - tMin;
    float stepSize = segmentLength / (float) FOG_MARCH_STEPS;
    float d = tMin + stepSize * jittering(float3(pixelPos, TIME));

    float3 accumulatedLight = float3(0.0f, 0.0f, 0.0f);
    float transmittance = 1.0f;

    float3 sunColor = get_dynamic_light_color(LIGHT_DIRECTION).rgb * 2.0f;
    float sundotrd = dot(rd, -LIGHT_DIRECTION);

    float3 ambientColor = FOG_COLOR * FOG_AMBIENT_STRENGTH;

    [loop]
    for (int i = 0; i < FOG_MARCH_STEPS; i++)
    {
        if (d > tMax)
            break;

        float3 p = ro + rd * d;

        float density = ComputeFogDensity(p, maxFogHeight);

        if (density > 0.0001f)
        {
            float shadow = GetFogShadow(p);
            float3 lightContrib = (sunColor * shadow) + ambientColor;

            float currentTransmittance = exp(-density * stepSize);
            float3 absorption = lightContrib * density * FOG_COLOR;
            
            accumulatedLight += transmittance * (absorption - absorption * currentTransmittance) / max(density, 0.0001f);
            transmittance *= currentTransmittance;
        }

        d += stepSize;

        if (transmittance <= 0.01f)
        {
            transmittance = 0.0f;
            break;
        }
    }
    
    float distanceFade = smoothstep(FOG_MAX_DISTANCE, FOG_MAX_DISTANCE * 0.8f, maxDist);
    accumulatedLight *= distanceFade;
    transmittance = lerp(1.0f, transmittance, distanceFade);

    return float4(accumulatedLight, transmittance);
} // RaymarchFog

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 scene = SceneTex.Sample(PointClampSampler, input.uv);
    float sceneDepth = DepthTex.Sample(PointClampSampler, input.uv).r;
    
    float3 ro = CAMERA_POSITION;
    float3 rd = ray_direction_restore(input.uv, PROJ_INV, VIEW_INV);
    
    float dist = FOG_MAX_DISTANCE;
    
    if (sceneDepth < 1.0f)
    {
        float3 worldPos = get_world_from_depth(input.uv, sceneDepth, VIEW_INV, PROJ_INV);
        dist = min(length(worldPos - ro), FOG_MAX_DISTANCE);
    }

    float4 fog = RaymarchFog(ro, rd, dist, (uint2) input.pos.xy);
    float3 normal = NormalMap.SampleLevel(PointClampSampler, input.uv, 0).xyz * 2.0f - 1.0f;
    normal = normalize(normal);
    
    // normal.y가 1이면 평지, 0이면 수직 절벽
    // fSlopeMin, fSlopeMax를 이용해 절벽에서 안개가 얼마나 걷혀보일지 부드럽게 제어
    float slopeFactor = smoothstep(0.2f, 0.8f, normal.y);
    
    // 절벽 부분은 투과율(fog.a)을 1(투명)에 가깝게 만들고, 산란광(fog.rgb)을 줄임
    fog.rgb *= lerp(0.1f, 1.0f, slopeFactor);
    fog.a = lerp(1.0f, fog.a, slopeFactor);
    
    float3 finalCol = scene.rgb * fog.a + fog.rgb;

    return float4(saturate(finalCol), 1.0f);
} // main