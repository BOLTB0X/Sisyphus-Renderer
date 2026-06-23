// WaterPS.hlsl
#include "Common.hlsli"

SamplerState LinearSampler : register(s0);
Texture2D    NormalMap1 : register(t1);
Texture2D    NormalMap2 : register(t2);
Texture2D    FlowMap : register(t3);
Texture2D    SceneDepthMap : register(t4);
Texture2D    SceneNormalMap : register(t5);
Texture2D    SceneMap : register(t6);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float2 uv : TEXCOORD0;
    float4 clipPos : TEXCOORD1;
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

float3 GetViewSpacePosition(float2 uv, float rawDepth)
{
    float4 clipSpacePos = float4(uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f, rawDepth, 1.0f);
    float4 viewSpacePos = mul(clipSpacePos, PROJ_INV);
    return viewSpacePos.xyz / viewSpacePos.w;
} // GetViewSpacePosition

float4 main(PS_IN input) : SV_TARGET
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
    // 현재 픽셀의 Screen UV 계산
    float2 screenUV = input.clipPos.xy / input.clipPos.w * 0.5f + 0.5f;
    screenUV.y = 1.0f - screenUV.y;

    // 수면 노말(파도) 계산 및 왜곡(Distortion) 생성
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
    float3 worldWaterNormal = normalize(float3(localNormal.x, localNormal.z, localNormal.y));

    float2 distortion = localNormal.xy * waterDistortion;
    float2 refractUV = saturate(screenUV + distortion);

    // 깊이(Depth) 차이 계산 (지형에 파묻히는 해안선 부드럽게 만들기)
    float rawSceneDepth = SceneDepthMap.SampleLevel(LinearSampler, screenUV, 0).r;
    float sceneLinearZ = GetViewSpacePosition(screenUV, rawSceneDepth).z;
    float waterLinearZ = input.clipPos.w; // Perspective에서 View Space Z는 W와 거의 동일

    float depthDiff = sceneLinearZ - waterLinearZ;

    // 만약 물 표면이 지형보다 깊다면
    if (depthDiff < 0.0f)
    {
        discard;
    }

    // 굴절(Refraction) 색상 및 수심에 따른 흡수(Beer's Law)
    float distortRawDepth = SceneDepthMap.SampleLevel(LinearSampler, refractUV, 0).r;
    float distortSceneZ = GetViewSpacePosition(refractUV, distortRawDepth).z;
    
    // 왜곡된 UV가 물 밖의 지형(하늘/바위)을 참조하면 왜곡 취소 (아티팩트 방지)
    if (distortSceneZ < waterLinearZ)
    {
        refractUV = screenUV;
    }

    float3 refractColor = SceneMap.Sample(LinearSampler, refractUV).rgb;
    float depthFactor = exp(-depthDiff * density);
    float3 waterColor = lerp(waterColorDeep, waterColorShallow, depthFactor);
    refractColor = lerp(waterColor, refractColor, depthFactor);

    // SSR (Screen Space Reflection) - 레이 마칭 루프
    float3 viewDir = normalize(input.worldPos - CAMERA_POSITION);
    float3 reflectDir = reflect(viewDir, worldWaterNormal);
    float3 viewReflectDir = mul(reflectDir, (float3x3) VIEW); // 레이 방향을 View 공간으로
    float3 viewPos = GetViewSpacePosition(screenUV, input.clipPos.z / input.clipPos.w); // 레이 시작점

    // 레이 마칭 세팅값 (성능과 퀄리티의 타협점)
    int maxSteps = 30;
    float stepSize = 0.5f;
    float thickness = 0.5f;

    float3 currentRayPos = viewPos;
    float2 hitUV = 0.0f;
    bool isHit = false;

    // SSR 루프
    [unroll(30)]
    for (int i = 0; i < maxSteps; ++i)
    {
        currentRayPos += viewReflectDir * stepSize;

        // 현재 레이 위치를 화면 UV로 투영
        float4 marchClipPos = mul(float4(currentRayPos, 1.0f), PROJ);
        float2 marchUV = marchClipPos.xy / marchClipPos.w * 0.5f + 0.5f;
        marchUV.y = 1.0f - marchUV.y;

        // 화면 밖으로 나가면 검사 종료
        if (marchUV.x < 0.0f || marchUV.x > 1.0f || marchUV.y < 0.0f || marchUV.y > 1.0f)
            break;

        // 현재 위치의 실제 씬 뎁스 샘플링
        float sampleRawDepth = SceneDepthMap.SampleLevel(LinearSampler, marchUV, 0).r;
        float sampleSceneZ = GetViewSpacePosition(marchUV, sampleRawDepth).z;

        float zDiff = currentRayPos.z - sampleSceneZ;

        // 레이가 오브젝트 뒤로 들어갔고(충돌), 두께 판정(너무 뒤의 물체를 반사하는 오류 방지) 안에 있다면
        if (zDiff > 0.0f && zDiff < thickness)
        {
            hitUV = marchUV;
            isHit = true;
            break;
        }
    }

    float3 reflectColor = float3(0, 0, 0);
    
    if (isHit)
    {
        // 충돌한 지점의 씬 색상(반사) 가져오기
        reflectColor = SceneMap.Sample(LinearSampler, hitUV).rgb;
    }
    else
    {
        // [FallBack] 레이가 화면 밖이나 하늘로 날아갔을 경우
        // 위쪽 하늘을 거꾸로 샘플링하여 가짜 반사 구현
        float2 skyUV = screenUV;
        skyUV.y = saturate(1.0f - screenUV.y + reflectDir.y);
        reflectColor = SceneMap.Sample(LinearSampler, skyUV).rgb;
    }

    // 프레넬 반사율 (Fresnel)
    float cosTheta = saturate(dot(-viewDir, worldWaterNormal));
    float fresnel = reflectivity + (1.0f - reflectivity) * pow(1.0f - cosTheta, 5.0f);

    // 최종 색상 합성
    float3 finalColor = lerp(refractColor, reflectColor, fresnel);

    // 해안선 처리용 알파 페이드 (물 깊이가 0에 가까우면 투명해짐)
    float edgeAlpha = saturate(depthDiff * 2.0f);

    return float4(finalColor, edgeAlpha);
}