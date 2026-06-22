// WaterPS.hlsl
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
    float3 worldPos : POSITION;
    float2 uv : TEXCOORD0;
    float4 reflectPosition : TEXCOORD1;
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

#define RESOLUTION          resolution

#define WATER_HEIGHT        waterHeight
#define WATER_COLOR_SHALLOW waterColorShallow
#define WATER_COLOR_DEEP    waterColorDeep
#define WATER_DISTORTION    waterDistortion
#define REFLECTIVITY        reflectivity
#define DENSITY             density

float4 main(PS_IN input) : SV_TARGET
{
    // 1. 투영 텍스처 매핑 (Projective Texture Mapping)
    // VS에서 넘어온 좌표를 w로 나누어 NDC 좌표(-1 ~ 1)로 만들고, UV 좌표(0 ~ 1)로 변환
    float2 reflectUV;
    reflectUV.x = input.reflectPosition.x / input.reflectPosition.w / 2.0f + 0.5f;
    reflectUV.y = -input.reflectPosition.y / input.reflectPosition.w / 2.0f + 0.5f;

    // 굴절 UV는 현재 메인 화면의 픽셀 좌표를 해상도로 나누어 그대로 사용
    float2 refractUV = input.pos.xy / RESOLUTION;

    // 2. 노말 맵 & 플로우 맵을 통한 물결 왜곡 (Distortion)
    float2 flowDir = FlowMap.Sample(LinearSampler, input.uv).rg * 2.0f - 1.0f;
    float flowSpeed = 0.03f;
    
    float phase0 = frac(TIME * flowSpeed);
    float phase1 = frac(TIME * flowSpeed + 0.5f);

    float2 uv0 = input.uv + flowDir * phase0;
    float2 uv1 = input.uv * 0.5f + flowDir * phase1;

    float3 n0 = NormalMap1.Sample(LinearSampler, uv0).rgb * 2.0f - 1.0f;
    float3 n1 = NormalMap2.Sample(LinearSampler, uv1).rgb * 2.0f - 1.0f;
    
    float blendFactor = abs((phase0 - 0.5f) * 2.0f);
    float3 localNormal = normalize(lerp(n0, n1, blendFactor));

    // 왜곡 강도 적용
    float2 distortion = localNormal.xy * WATER_DISTORTION;
    reflectUV = saturate(reflectUV + distortion);
    refractUV = saturate(refractUV + distortion * 0.5f);

    // 3. 굴절 깊이 아티팩트 방지 (물 밖을 왜곡하는 현상 막기)
    float rawDepth = SceneDepthMap.SampleLevel(LinearSampler, refractUV, 0).r;
    float distortLinearDepth = depth_to_meter(rawDepth, PROJ);
    
    // 카메라에서 픽셀까지의 실제 뷰 스페이스 Z거리
    float waterLinearDepth = input.pos.w;
    
    // 왜곡된 곳의 깊이가 현재 물 메쉬의 깊이보다 카메라에 가깝다면 (즉, 수면 위 허공/지형이라면)
    if (distortLinearDepth < waterLinearDepth)
    {
        // 왜곡 취소
        refractUV = input.pos.xy / RESOLUTION;
    }

    // 4. 색상 샘플링 및 Beer's Law 깊이 흡수
    float3 reflectColor = ReflectionMap.Sample(LinearSampler, reflectUV).rgb;
    float3 refractColor = SceneMap.Sample(LinearSampler, refractUV).rgb;

    float actualWaterDepth = max(distortLinearDepth - waterLinearDepth, 0.0f);
    float depthFactor = exp(-actualWaterDepth * density);
    float3 waterColor = lerp(waterColorDeep, waterColorShallow, depthFactor);

    refractColor = lerp(waterColor, refractColor, depthFactor);

    // 5. 프레넬 반사율 (Fresnel)
    float3 worldNormal = normalize(float3(localNormal.x, localNormal.z, localNormal.y));
    float3 viewDir = normalize(CAMERA_POSITION - input.worldPos);
    
    float cosTheta = saturate(dot(viewDir, worldNormal));
    float fresnel = reflectivity + (1.0f - reflectivity) * pow(1.0f - cosTheta, 5.0f);

    // 최종 반사 + 굴절 합성
    float3 finalColor = lerp(refractColor, reflectColor, fresnel);

    //return float4(1.0f, 0.0f, 1.0f, 1.0f);
    return float4(finalColor, 1.0f);
}