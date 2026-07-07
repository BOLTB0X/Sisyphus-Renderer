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
Texture3D        WorleyNoise : register(t4);
Texture2D        NormalMap : register(t5);
Texture2D        TerrainShadowMap : register(t11);

cbuffer VolumetricFogBuffer : register(b2)
{
    // Row 1
    float fBaseHeight;
    float fHeightFalloff;
    float fDensity;
    float fMaxDistance;
    // Row 2
    float fNoiseScale;
    float fNoiseStrength;
    float fWindSpeed;
    float fFogPhaseG;
    // Row 3
    float3 fColor;
    float fAmbientStrength;
    // Row 4
    float2 fWindDirection;
    float fLightMarchDist;
    int fLightMarchSteps;
    // Row 5: 가두는 구(Sphere)
    float3 fFogSphereCenter;
    float fFogSphereRadius;
    // Row 6
    float fFogEdgeSoftness;
    int fMarchSteps;
    float2 vPadding;
} // VolumetricFogBuffer

#define FOG_BASE_HEIGHT       fBaseHeight
#define FOG_HEIGHT_FALLOFF    fHeightFalloff
#define FOG_DENSITY           fDensity
#define FOG_MAX_DISTANCE      fMaxDistance
#define FOG_NOISE_SCALE       fNoiseScale
#define FOG_NOISE_STRENGTH    fNoiseStrength
#define FOG_WIND_SPEED        fWindSpeed
#define FOG_PHASE_G           fFogPhaseG
#define FOG_LIGHT_MARCH_STEPS fLightMarchSteps
#define FOG_LIGHT_MARCH_DIST  fLightMarchDist
#define FOG_COLOR             fColor
#define FOG_AMBIENT_STRENGTH  fAmbientStrength
#define FOG_WIND_DIRECTION    fWindDirection
#define FOG_SPHERE_CENTER     fFogSphereCenter
#define FOG_SPHERE_RADIUS     fFogSphereRadius
#define FOG_EDGE_SOFTNESS     fFogEdgeSoftness
#define FOG_MARCH_STEPS       fMarchSteps

float ComputeFogDensitySphere(float3 p)
{
    float distFromCenter = length(p - FOG_SPHERE_CENTER);

    float sphereFalloff = exp(-pow(distFromCenter / FOG_SPHERE_RADIUS, 2.0f));

    float heightAboveBase = max((p.y - FOG_SPHERE_CENTER.y) - FOG_BASE_HEIGHT, 0.0f);
    float heightFalloff = exp(-heightAboveBase * FOG_HEIGHT_FALLOFF);

    float2 noiseCoord = p.xz * (FOG_NOISE_SCALE * 0.1f);
    noiseCoord += FOG_WIND_DIRECTION * TIME * FOG_WIND_SPEED;
    float baseNoise = NoiseMap.SampleLevel(LinearWrapSampler, noiseCoord, 0).r;

    float3 worleyCoord = p * FOG_NOISE_SCALE;
    float detailNoise = WorleyNoise.SampleLevel(LinearWrapSampler, worleyCoord, 0).r;

    float combinedNoise = baseNoise * detailNoise;
    float noiseFactor = lerp(1.0f, combinedNoise, FOG_NOISE_STRENGTH);
    noiseFactor = max(noiseFactor, 0.1f);

    return FOG_DENSITY * sphereFalloff * heightFalloff * noiseFactor;
} // ComputeFogDensitySphere

float GetFogShadow(float3 worldPos)
{
    float4 lightClipPos = mul(mul(float4(worldPos, 1.0f), LIGHT_VIEW), LIGHT_PROJ);
    return calculate_poisson_shadow(ShadowSampler, TerrainShadowMap, lightClipPos,
        SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);
} // GetFogShadow

float RaymarchLight(float3 p)
{
    float stepSize = FOG_LIGHT_MARCH_DIST / (float) FOG_LIGHT_MARCH_STEPS;
    float3 dir = -LIGHT_DIRECTION;
    float3 pos = p;

    float opticalDepth = 0.0f;

    [loop]
    for (int i = 0; i < FOG_LIGHT_MARCH_STEPS; i++)
    {
        pos += dir * stepSize;

        float distFromCenter = length(pos - FOG_SPHERE_CENTER);
        if (distFromCenter > FOG_SPHERE_RADIUS)
            break;

        float density = ComputeFogDensitySphere(pos);
        opticalDepth += density * stepSize * 4.0f;
    }

    return exp(-opticalDepth);
} // RaymarchLight

float4 RaymarchFog(float3 ro, float3 rd, float maxDist, uint2 pixelPos)
{
    if (maxDist <= 0.0f)
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    float tMin = 0.0f;
    float tMax = maxDist;
    float stepSize = tMax / (float) FOG_MARCH_STEPS;
    float d = tMin + stepSize * jittering(float3(pixelPos, TIME));

    float3 accumulatedLight = float3(0.0f, 0.0f, 0.0f);
    float transmittance = 1.0f;

    float3 sunColor = get_dynamic_light_color(LIGHT_DIRECTION).rgb;
    
    float sundotrd = dot(rd, -LIGHT_DIRECTION);

    float phaseCore = henyey_greenstein(sundotrd, FOG_PHASE_G);
    float phaseBroad = henyey_greenstein(sundotrd, FOG_PHASE_G * 0.3f);
    float phase = lerp(phaseBroad, phaseCore, 0.5f);

    phase = min(phase, 2.0f);

    float3 ambientColor = FOG_COLOR * FOG_AMBIENT_STRENGTH;
    float scatteringIntensity = 0.2f;

    [loop]
    for (int i = 0; i < FOG_MARCH_STEPS; i++)
    {
        if (d > tMax)
            break;

        float3 p = ro + rd * d;
        float density = ComputeFogDensitySphere(p);

        if (density > 0.0001f)
        {
            float shadowMapOcclusion = GetFogShadow(p);
            float lightTransmittance = RaymarchLight(p);
            float3 lightContrib = (sunColor * shadowMapOcclusion * lightTransmittance * phase) + ambientColor;

            float currentTransmittance = exp(-density * stepSize);
            float3 absorption = lightContrib * density * FOG_COLOR * scatteringIntensity;

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
    float3 finalCol = scene.rgb * fog.a + fog.rgb;

    return float4(saturate(finalCol), 1.0f);
} // main