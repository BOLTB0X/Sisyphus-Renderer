// WaterCompositePS.hlsl
// https://www.rastertek.com/dx11win10tut31.html
#include "Common.hlsli"

Texture2D NormalMap1 : register(t1);
Texture2D NormalMap2 : register(t2);
Texture2D FlowMap : register(t3);
Texture2D SceneDepthMap : register(t4);
Texture2D ReflectionMap : register(t5);
Texture2D SceneMap : register(t6);

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
    float sceneLinearDepth = depth_to_meter(rawDepth, PROJ);

    float3 ro = CAMERA_POSITION;
    float3 rd = ray_direction_restore(input.uv, PROJ_INV, VIEW_INV);

    float t = -(ro.y - WATER_HEIGHT) / rd.y;

    // 물을 그려야 하는 조건 검사
    // t > 0: 수면이 카메라 앞쪽에 있음 (하늘 위로 쏜 광선 무시)
    // t < sceneLinearDepth: 물 평면이 지형(바닥)보다 카메라에 가까움 (물에 잠긴 지형)
    if (t > 0.0f && t < sceneLinearDepth)
    {
        // 광선이 물 평면에 닿은 정확한 월드 좌표
        float3 waterWorldPos = ro + rd * t;

        // 메쉬의 UV 대신, 월드 좌표의 XZ 평면을 UV로 활용하여 타일링
        float2 waterUV = waterWorldPos.xz * 0.01f;

        // -----------------------------------------------------------
        // 물결 왜곡
        // -----------------------------------------------------------
        float2 flowDir = FlowMap.Sample(LinearSampler, waterUV).rg * 2.0f - 1.0f;
        float flowSpeed = 0.03f;

        float phase0 = frac(TIME * flowSpeed);
        float phase1 = frac(TIME * flowSpeed + 0.5f);

        float2 uv0 = waterUV * 4.0f + flowDir * phase0;
        float2 uv1 = waterUV * 2.5f + flowDir * phase1;

        float3 n0 = NormalMap1.Sample(LinearSampler, uv0).rgb * 2.0f - 1.0f;
        float3 n1 = NormalMap2.Sample(LinearSampler, uv1).rgb * 2.0f - 1.0f;
        
        float blendFactor = abs((phase0 - 0.5f) * 2.0f);
        float3 localNormal = normalize(lerp(n0, n1, blendFactor));

        // 화면 공간 왜곡 강도 계산
        float2 distortion = localNormal.xy * WATER_DISTORTION;

        // 왜곡된 화면 UV (saturate로 화면 밖을 참조하지 않도록 보호)
        float2 reflectUV = saturate(input.uv + distortion);
        float2 refractUV = saturate(input.uv + distortion * 0.5f);

        // [아티팩트 방지] 굴절 왜곡 UV가 물 밖의 지형이나 허공을 참조하면 원본 UV로 롤백
        float distortRawDepth = SceneDepthMap.SampleLevel(LinearSampler, refractUV, 0).r;
        float distortLinearDepth = depth_to_meter(distortRawDepth, PROJ);
        if (distortLinearDepth < t)
        {
            refractUV = input.uv;
        }

        // 월드 노멀 및 시선 벡터
        float3 worldNormal = normalize(float3(localNormal.x, localNormal.z, localNormal.y));
        float3 viewDir = -rd; // 시선 방향은 카메라 광선의 반대

        // -----------------------------------------------------------
        // Beer's Law 깊이 색상 블렌딩
        // -----------------------------------------------------------
        // 실제 물의 깊이 = (바닥까지의 거리) - (수면까지의 거리)
        float actualWaterDepth = sceneLinearDepth - t;
        float depthFactor = exp(-actualWaterDepth * WATER_DENSITY);
        float3 waterColor = lerp(WATER_COLOR_DEEP, WATER_COLOR_SHALLOW, depthFactor);

        // -----------------------------------------------------------
        // 텍스처 합성 및 Fresnel 적용
        // -----------------------------------------------------------
        float3 reflectColor = ReflectionMap.Sample(LinearSampler, reflectUV).rgb;
        
        // 굴절 맵은 따로 없고, SceneMap(메인 씬) 자체가 왜곡되는 굴절 역할을 함!
        float3 refractColor = SceneMap.Sample(LinearSampler, refractUV).rgb;

        // 바닥(굴절) 색상에 물의 색상을 흡수시킴
        refractColor = lerp(waterColor, refractColor, depthFactor);

        // 프레넬 반사율 계산
        float cosTheta = saturate(dot(viewDir, worldNormal));
        float fresnel = WATER_REFLECTIVITY + (1.0f - WATER_REFLECTIVITY) * pow(1.0f - cosTheta, 5.0f);

        // 최종 반사와 굴절 혼합
        float3 finalColor = lerp(refractColor, reflectColor, fresnel);

        return float4(finalColor, 1.0f);
    }
    return float4(originalSceneColor, 1.0f);
} // main