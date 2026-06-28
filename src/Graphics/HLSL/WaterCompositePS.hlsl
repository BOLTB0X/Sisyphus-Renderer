// WaterCompositePS.hlsl
// https://www.rastertek.com/dx11win10tut31.html
// https://www.shadertoy.com/view/3lyXRt
// https://www.shadertoy.com/view/Xl2XRW
// https://www.shadertoy.com/view/Ms2SD1
#include "Common.hlsli"
#include "ShadowMap.hlsli"

SamplerState           LinearSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s5);

Texture2D    NormalMap1 : register(t1);
Texture2D    NormalMap2 : register(t2);
Texture2D    SceneDepthMap : register(t3);
Texture2D    SceneMap : register(t4);
Texture2D    SceneNormalMap : register(t5);
Texture2D    ObjectShadowMap : register(t10);
Texture2D    TerrainShadowMap : register(t11);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
}; // PS_IN

cbuffer ResolutionBuffer : register(b2)
{
    float2 resolution;
    float2 resPadding;
}; // ResolutionBuffer

cbuffer WaterBuffer : register(b3)
{
    float waterHeight;
    float3 wPadding1;
    
    float3 waterColorShallow;
    float wPadding2;
    
    float3 waterColorDeep;
    float wPadding3;
    
    float waterDistortion;
    float reflectivity;
    float density;
    float sunShininess;
    
    float2 lightUV;
    float2 wPadding4;
    
    int raymarchMaxStep;
    float stepSize;
    float thickness;
    float wPadding5;
}; // WaterBuffer

#define WATER_HEIGHT        waterHeight
#define WATER_COLOR_SHALLOW waterColorShallow
#define WATER_COLOR_DEEP    waterColorDeep
#define WATER_DISTORTION    waterDistortion
#define WATER_REFLECTIVITY  reflectivity
#define WATER_DENSITY       density
#define SUN_SHININESS       sunShininess
#define LIGHT_UV            lightUV
#define RAY_MAX_STEP_SIZE   raymarchMaxStep
#define STEP_SIZE           stepSize
#define THICKNESS           thickness

void CalculateWaterNormal(float2 waterUV, out float3 localNormal, out float3 worldNormal)
{
    float2 uv0 = waterUV * 4.0f + float2(TIME * 0.015f, TIME * 0.01f);
    float2 uv1 = waterUV * 2.5f + float2(-TIME * 0.01f, TIME * 0.02f);

    float3 n0 = NormalMap1.Sample(LinearSampler, uv0).rgb * 2.0f - 1.0f;
    float3 n1 = NormalMap2.Sample(LinearSampler, uv1).rgb * 2.0f - 1.0f;
    
    localNormal = normalize(n0 + n1);
    worldNormal = normalize(float3(localNormal.x, localNormal.z, localNormal.y));
} // CalculateWaterNormal

float3 RaymarchSSR(float3 waterWorldPos, float3 worldNormal, float3 viewDir, float2 screenUV)
{
    float3 reflectDir = reflect(-viewDir, worldNormal);
    float3 viewReflectDir = normalize(mul(reflectDir, (float3x3) VIEW));
    float3 viewPos = mul(float4(waterWorldPos, 1.0f), VIEW).xyz;

    float3 currentRayPos = viewPos;
    float2 hitUV = 0.0f;
    bool isHit = false;

    [loop] 
    for (int i = 0; i < RAY_MAX_STEP_SIZE; ++i)
    {
        currentRayPos += viewReflectDir * STEP_SIZE;
        float4 marchClipPos = mul(float4(currentRayPos, 1.0f), PROJ);
        float2 marchUV = marchClipPos.xy / marchClipPos.w * 0.5f + 0.5f;
        marchUV.y = 1.0f - marchUV.y;

        if (marchUV.x < 0.0f || marchUV.x > 1.0f || marchUV.y < 0.0f || marchUV.y > 1.0f)
            break;

        float sampleRawDepth = SceneDepthMap.SampleLevel(LinearSampler, marchUV, 0).r;
        float sampleSceneZ = depth_to_meter(sampleRawDepth, PROJ);
        float zDiff = currentRayPos.z - sampleSceneZ;

        if (zDiff > 0.0f && zDiff < THICKNESS)
        {
            float3 hitNormal = SceneNormalMap.SampleLevel(LinearSampler, marchUV, 0).rgb * 2.0f - 1.0f;
            if (dot(viewReflectDir, hitNormal) > 0.0f)
            {
                continue;
            }

            hitUV = marchUV;
            isHit = true;
            break;
        }
    }

    if (isHit)
    {
        return SceneMap.SampleLevel(LinearSampler, hitUV, 0).rgb;
    }
    else
    {
        float2 skyUV = screenUV;
        skyUV.y = saturate(1.0f - screenUV.y + reflectDir.y * 0.5f);
        return SceneMap.SampleLevel(LinearSampler, skyUV, 0).rgb;
    }
} // RaymarchSSR


float3 GetSunHighlight(float2 screenUV, float3 worldNormal, float3 localNormal, float3 viewDir)
{
    float3 lightDir = normalize(-LIGHT_DIRECTION);
    float3 halfVector = normalize(lightDir + viewDir);
    float NdotH = max(0.0f, dot(worldNormal, halfVector));
    float specular = pow(NdotH, SUN_SHININESS);

    float sunPillarGlow = 0.0f;
    if (LIGHT_UV.x > 0.0f && LIGHT_UV.x < 1.0f && LIGHT_UV.y > 0.0f)
    {
        float depthWeight = saturate(screenUV.y - LIGHT_UV.y);
        float rippleOffset = localNormal.x * 0.04f * depthWeight;
        float distToSunX = abs(screenUV.x + rippleOffset - LIGHT_UV.x);
        float pillarWidth = lerp(0.01f, 0.08f, depthWeight);
        
        float glowLine = smoothstep(pillarWidth, 0.0f, distToSunX) * smoothstep(1.0f, LIGHT_UV.y, screenUV.y);
        float sparkle = saturate(localNormal.y * 1.5f + 0.2f);
        
        sunPillarGlow = glowLine * sparkle;
    }

    return LIGHT_COLOR.rgb * (specular * 3.0f + sunPillarGlow * 2.0f);
} // GetSunHighlight

float GetWaterShadow(float3 worldPos)
{
    // 지형 그림자
    float4 lightClipPos = mul(mul(float4(worldPos, 1.0f), LIGHT_VIEW), LIGHT_PROJ);
    float terrainShadow = calculate_poisson_shadow(ShadowSampler, TerrainShadowMap, lightClipPos, SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);
    
    // 객체 그림자
    float4 objLightClipPos = mul(mul(float4(worldPos, 1.0f), LIGHT_OBJECT_VIEW), LIGHT_OBJECT_PROJ);
    float objectShadow = calculate_poisson_shadow(ShadowSampler, ObjectShadowMap, objLightClipPos, SHADOW_MAP_SIZE, SHADOW_SPREAD, SHADOW_BIAS);
    
    return min(terrainShadow, objectShadow);
} // GetWaterShadow

float4 main(PS_IN input) : SV_TARGET
{
    float3 originalSceneColor = SceneMap.SampleLevel(LinearSampler, input.uv, 0).rgb;
    float rawDepth = SceneDepthMap.SampleLevel(LinearSampler, input.uv, 0).r;
    
    float4 clipSpacePos = float4(input.uv.x * 2.0f - 1.0f, 1.0f - input.uv.y * 2.0f, rawDepth, 1.0f);
    float4 viewSpacePos = mul(clipSpacePos, PROJ_INV);
    viewSpacePos.xyz /= viewSpacePos.w;
    
    float sceneActualDistance = length(viewSpacePos.xyz);

    float3 ro = CAMERA_POSITION;
    float3 rd = ray_direction_restore(input.uv, PROJ_INV, VIEW_INV);
    float t = -(ro.y - WATER_HEIGHT) / rd.y;
    
    // 교차점이 존재하고, 지형보다 카메라에 가까울 때만 물을 그림
    if (t > 0.0f && t < sceneActualDistance)
    {
        float3 waterWorldPos = ro + rd * t;
        float2 waterUV = waterWorldPos.xz * 0.01f;
        float3 viewDir = -rd;

        // 파도 노말 연산
        float3 localNormal, worldNormal;
        CalculateWaterNormal(waterUV, localNormal, worldNormal);

        // 굴절(Refraction) 색상 연산
        float2 distortion = localNormal.xy * WATER_DISTORTION;
        float2 refractUV = saturate(input.uv + distortion * 0.5f);
        
        float distortRawDepth = SceneDepthMap.SampleLevel(LinearSampler, refractUV, 0).r;
        if (depth_to_meter(distortRawDepth, PROJ) < t)
        {
            refractUV = input.uv;
        }

        float actualWaterDepth = sceneActualDistance - t;
        float depthFactor = exp(-actualWaterDepth * WATER_DENSITY);
        float3 waterColor = lerp(WATER_COLOR_DEEP, WATER_COLOR_SHALLOW, depthFactor);

        float3 refractColor = SceneMap.SampleLevel(LinearSampler, refractUV, 0).rgb;
        refractColor = lerp(waterColor, refractColor, depthFactor);

        float3 reflectColor = RaymarchSSR(waterWorldPos, worldNormal, viewDir, input.uv);

        float3 sunHighlight = GetSunHighlight(input.uv, worldNormal, localNormal, viewDir);
        float shadowFactor = GetWaterShadow(waterWorldPos);
        sunHighlight *= shadowFactor;

        float cosTheta = saturate(dot(viewDir, worldNormal));
        float fresnel = WATER_REFLECTIVITY + (1.0f - WATER_REFLECTIVITY) * pow(1.0f - cosTheta, 5.0f);

        float3 waterLitColor = lerp(refractColor, reflectColor, fresnel);
        float3 finalColor = waterLitColor * lerp(0.8f, 1.0f, shadowFactor) + sunHighlight;
        
        return float4(finalColor, 1.0f);
    }

    return float4(originalSceneColor, 1.0f);
} // main