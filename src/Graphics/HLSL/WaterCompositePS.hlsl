// WaterCompositePS.hlsl
// https://www.rastertek.com/dx11win10tut31.html
// https://www.shadertoy.com/view/3lyXRt
#include "Common.hlsli"

Texture2D NormalMap1 : register(t1);
Texture2D NormalMap2 : register(t2);
Texture2D SceneDepthMap : register(t3);
Texture2D SceneMap : register(t4);
Texture2D SceneNormalMap : register(t5);

SamplerState LinearSampler : register(s0);

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
    float  waterHeight;
    float3 wPadding1;
    
    float3 waterColorShallow;
    float  wPadding2;
    
    float3 waterColorDeep;
    float  wPadding3;
    
    float  waterDistortion;
    float  reflectivity;
    float  density;
    float  wPadding4;
}; // WaterBuffer

#define WATER_HEIGHT        waterHeight
#define WATER_COLOR_SHALLOW waterColorShallow
#define WATER_COLOR_DEEP    waterColorDeep
#define WATER_DISTORTION    waterDistortion
#define WATER_REFLECTIVITY  reflectivity
#define WATER_DENSITY       density

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

        float2 uv0 = waterUV * 4.0f + float2(TIME * 0.015f, TIME * 0.01f);
        float2 uv1 = waterUV * 2.5f + float2(-TIME * 0.01f, TIME * 0.02f); // 방향 다르게

        float3 n0 = NormalMap1.Sample(LinearSampler, uv0).rgb * 2.0f - 1.0f;
        float3 n1 = NormalMap2.Sample(LinearSampler, uv1).rgb * 2.0f - 1.0f;
        
        float3 localNormal = normalize(n0 + n1);

        float2 distortion = localNormal.xy * WATER_DISTORTION;
        float2 refractUV = saturate(input.uv + distortion * 0.5f);

        float distortRawDepth = SceneDepthMap.SampleLevel(LinearSampler, refractUV, 0).r;
        float distortLinearDepth = depth_to_meter(distortRawDepth, PROJ);
        
        // 굴절된 UV가 물 밖의 지형을 참조하면 아티팩트 방지
        if (distortLinearDepth < t)
        {
            refractUV = input.uv;
        }

        float3 worldNormal = normalize(float3(localNormal.x, localNormal.z, localNormal.y));
        float3 viewDir = -rd;

        // 수심에 따른 색상 흡수 (Refraction & Beer's Law)
        float actualWaterDepth = sceneActualDistance - t;
        float depthFactor = exp(-actualWaterDepth * WATER_DENSITY);
        float3 waterColor = lerp(WATER_COLOR_DEEP, WATER_COLOR_SHALLOW, depthFactor);

        float3 refractColor = SceneMap.SampleLevel(LinearSampler, refractUV, 0).rgb;
        refractColor = lerp(waterColor, refractColor, depthFactor);

        float3 reflectDir = reflect(rd, worldNormal);
        float3 viewReflectDir = normalize(mul(reflectDir, (float3x3) VIEW));
        float3 viewPos = mul(float4(waterWorldPos, 1.0f), VIEW).xyz; // 물 표면의 View 공간 좌표

        int maxSteps = 30; // 레이 반복 횟수 (품질)
        float stepSize = 0.5f; // 한 스텝당 전진 거리
        float thickness = 0.5f; // 뒤쪽 오브젝트 오판 방지 두께

        float3 currentRayPos = viewPos;
        float2 hitUV = 0.0f;
        bool isHit = false;

        [unroll(30)]
        for (int i = 0; i < maxSteps; ++i)
        {
            currentRayPos += viewReflectDir * stepSize;

            // View -> Screen UV 투영
            float4 marchClipPos = mul(float4(currentRayPos, 1.0f), PROJ);
            float2 marchUV = marchClipPos.xy / marchClipPos.w * 0.5f + 0.5f;
            marchUV.y = 1.0f - marchUV.y;

            // 화면 밖으로 나가면 즉시 포기
            if (marchUV.x < 0.0f || marchUV.x > 1.0f || marchUV.y < 0.0f || marchUV.y > 1.0f)
                break;

            // 현재 UV의 깊이를 가져와 선형 View Z로 변환
            float sampleRawDepth = SceneDepthMap.SampleLevel(LinearSampler, marchUV, 0).r;
            float sampleSceneZ = depth_to_meter(sampleRawDepth, PROJ);

            float zDiff = currentRayPos.z - sampleSceneZ;

            if (zDiff > 0.0f && zDiff < thickness)
            {
                float3 hitNormal = SceneNormalMap.SampleLevel(LinearSampler, marchUV, 0).rgb * 2.0f - 1.0f;
                
                // 레이 방향과 노말 방향 내적
                // 내적 값이 양수라면, 레이가 물체의 뒷면을 뚫고 들어갔다는 뜻
                if (dot(viewReflectDir, hitNormal) > 0.0f)
                {
                    continue;
                }

                hitUV = marchUV;
                isHit = true;
                break;
            }
        }

        float3 reflectColor = float3(0, 0, 0);

        if (isHit)
        {
            // 지형이나 돌에 부딪힌 경우 (SSR)
            reflectColor = SceneMap.SampleLevel(LinearSampler, hitUV, 0).rgb;
        }
        else
        {
            float2 skyUV = input.uv;
            skyUV.y = saturate(1.0f - input.uv.y + reflectDir.y * 0.5f);
            reflectColor = SceneMap.SampleLevel(LinearSampler, skyUV, 0).rgb;
        }

        // 프레넬 연산 및 최종 합성
        float cosTheta = saturate(dot(viewDir, worldNormal));
        float fresnel = WATER_REFLECTIVITY + (1.0f - WATER_REFLECTIVITY) * pow(1.0f - cosTheta, 5.0f);

        float3 finalColor = lerp(refractColor, reflectColor, fresnel);
        
        return float4(finalColor, 1.0f);
    }

    return float4(originalSceneColor, 1.0f);
} // main