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
    float3 reflectDir = reflect(-viewDir, worldNormal); // 월드 공간 반사 벡터
    
    // 뷰 공간으로 변환
    float3 viewReflectDir = normalize(mul(reflectDir, (float3x3) VIEW));
    float3 viewPos = mul(float4(waterWorldPos, 1.0f), VIEW).xyz;

    float3 currentRayPos = viewPos;
    float2 hitUV = 0.0f;
    bool isHit = false;

    float adaptiveThickness = max(STEP_SIZE * 1.5f, THICKNESS);

    [loop] 
    for (int i = 0; i < RAY_MAX_STEP_SIZE; ++i)
    {
        // 뷰 공간에서 레이 전진
        currentRayPos += viewReflectDir * STEP_SIZE;
        
        // 투영 공간 및 화면 UV 변환
        float4 marchClipPos = mul(float4(currentRayPos, 1.0f), PROJ);
        float2 marchUV = marchClipPos.xy / marchClipPos.w * 0.5f + 0.5f;
        marchUV.y = 1.0f - marchUV.y;

        // 화면 영역을 벗어나면 루프 탈출
        if (marchUV.x < 0.0f || marchUV.x > 1.0f || marchUV.y < 0.0f || marchUV.y > 1.0f)
            break;

        // 씬 깊이 버퍼 샘플링 후 뷰 공간 선형 깊이(Z)로 변환
        float sampleRawDepth = SceneDepthMap.SampleLevel(LinearSampler, marchUV, 0).r;
        float sampleSceneZ = depth_to_meter(sampleRawDepth, PROJ);
        
        // 레이의 현재 깊이와 실제 지형 깊이 비교
        float zDiff = currentRayPos.z - sampleSceneZ;

        // 적응형 두께 판정 구간 진입
        if (zDiff > 0.0f && zDiff < adaptiveThickness)
        {
            // 월드 공간 지형 노말 샘플링
            float3 hitNormal = SceneNormalMap.SampleLevel(LinearSampler, marchUV, 0).rgb * 2.0f - 1.0f;
            hitNormal = normalize(hitNormal);

            if (dot(reflectDir, hitNormal) > 0.0f)
            {
                continue; // 카메라를 등진 뒷면 지형 파편이라면 충돌 스킵
            }

            currentRayPos -= viewReflectDir * STEP_SIZE * 0.5f;
            marchClipPos = mul(float4(currentRayPos, 1.0f), PROJ);
            marchUV = marchClipPos.xy / marchClipPos.w * 0.5f + 0.5f;
            marchUV.y = 1.0f - marchUV.y;

            hitUV = marchUV;
            isHit = true;
            break;
        }
    }

    if (isHit)
    {
        // 충돌한 씬의 색상 반환
        return SceneMap.SampleLevel(LinearSampler, hitUV, 0).rgb;
    }
    else
    {
        float2 skyUV = screenUV;
        skyUV.y = saturate(1.0f - screenUV.y + reflectDir.y * 0.5f);
        return SceneMap.SampleLevel(LinearSampler, skyUV, 0).rgb;
    }
} // RaymarchSSR

float3 RaymarchSSR2(float3 waterWorldPos, float3 worldNormal, float3 viewDir, float2 screenUV)
{
    float3 reflectDir = reflect(-viewDir, worldNormal);
    
    if (reflectDir.y < 0.0f)
    {
        reflectDir.y = -reflectDir.y;
        reflectDir = normalize(reflectDir);
    }

    float3 viewPos = mul(float4(waterWorldPos, 1.0f), VIEW).xyz;
    float3 viewReflectDir = normalize(mul(reflectDir, (float3x3) VIEW));

    float stepSize = 0.5f;
    float3 currentRayPos = viewPos;
    bool isHit = false;
    float2 hitUV = 0.0f;
    float rayDistance = 0.0f;

    [loop]
    for (int i = 0; i < RAY_MAX_STEP_SIZE; ++i)
    {
        currentRayPos += viewReflectDir * stepSize;
        rayDistance += stepSize;

        // 현재 레이 위치를 화면 UV로 변환
        float4 clipPos = mul(float4(currentRayPos, 1.0f), PROJ);
        if (clipPos.w < 0.0001f)
            break; // 카메라 뒤로 넘어감

        float2 marchUV = clipPos.xy / clipPos.w * 0.5f + 0.5f;
        marchUV.y = 1.0f - marchUV.y;

        // 화면 밖으로 나가면 탐색 종료
        if (marchUV.x < 0.0f || marchUV.x > 1.0f || marchUV.y < 0.0f || marchUV.y > 1.0f)
            break;

        float rawDepth = SceneDepthMap.SampleLevel(LinearSampler, marchUV, 0).r;
        
        if (rawDepth >= 0.9999f) 
            break;

        float sceneZ = depth_to_meter(rawDepth, PROJ);
        float zDiff = currentRayPos.z - sceneZ;
        float adaptiveThickness = 0.2f + rayDistance * 0.05f;

        if (zDiff > 0.0f && zDiff < adaptiveThickness)
        {
            // 카메라를 등진 뒷면 지형인지 확인
            float3 hitNormal = SceneNormalMap.SampleLevel(LinearSampler, marchUV, 0).rgb * 2.0f - 1.0f;
            hitNormal = normalize(hitNormal);
            if (dot(reflectDir, hitNormal) <= 0.0f)
            {
                hitUV = marchUV;
                isHit = true;
                break;
            }
        }
        stepSize *= 1.1f;
    }
    
    if (isHit)
    {
        return SceneMap.SampleLevel(LinearSampler, hitUV, 0).rgb;
    }
    else
    {
        float2 skyUV = screenUV;
        skyUV.y = clamp(0.2f - reflectDir.y * 0.15f, 0.05f, 0.4f);
        
        return SceneMap.SampleLevel(LinearSampler, skyUV, 0).rgb;
    }
} // RaymarchSSR2

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
        
        float3 ssrNormal = normalize(lerp(float3(0.0f, 1.0f, 0.0f), worldNormal, 0.3f));
        float3 reflectColor = RaymarchSSR2(waterWorldPos, ssrNormal, viewDir, input.uv);

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