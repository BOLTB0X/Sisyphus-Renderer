// WaterPS.hlsl
// https://www.rastertek.com/dx11win10tut31.html
#include "Common.hlsli"

Texture2D NormalMap1 : register(t0); // water_normal.jpg
Texture2D NormalMap2 : register(t1); // water_wave_normal.jpg
Texture2D FlowMap : register(t2); // flowmap.png
Texture2D ReflectionMap : register(t3); // RTT
Texture2D RefractionMap : register(t4); // RTT
Texture2D SceneDepthMap : register(t5); // Main Scene Depth

SamplerState LinearSampler : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION; // screen space (x, y, depth, 1/w)
    float2 uv : TEXCOORD0; // 타일링 UV
    float4 clipSpace : TEXCOORD1; // VS에서 넘겨준 Clip Space 좌표
}; // PS_IN

cbuffer WaterBuffer : register(b2)
{
    matrix worldMatrix;
    
    float waterHeight;
    float3 wPadding1;
    
    float3 waterColorShallow;
    float wPadding2;
    
    float3 waterColorDeep;
    float wPadding3;
    
    float waterDistortion;
    float reflectivity;
    float density;
    float wPadding4;
}; // WaterBuffer

#define WATER_COLOR_SHALLOW waterColorShallow
#define WATER_COLOR_DEEP    waterColorDeep
#define WATER_DISTORTION    waterDistortion 
#define WATER_REFLECTIVITY  reflectivity 
#define WATER_DENSITY       density

float4 main(PS_IN input) : SV_TARGET
{
    float2 ndc = (input.clipSpace.xy / input.clipSpace.w);
    float2 screenUV = ndc * 0.5f + 0.5f;
    screenUV.y = 1.0f - screenUV.y;


    // 플로우 맵 방향 추출 (-1 ~ 1 범위 리맵핑)
    float2 flowDir = FlowMap.Sample(LinearSampler, input.uv).rg * 2.0f - 1.0f;
    float flowSpeed = 0.03f;

    // 텍스처 찢어짐 방지용 2주기 페이즈
    float phase0 = frac(TIME * flowSpeed);
    float phase1 = frac(TIME * flowSpeed + 0.5f);

    // 흐름 방향으로 각각 panning 하는 UV
    float2 uv0 = input.uv * 4.0f + flowDir * phase0; // 물결 타일링을 위해 기본 UV에 배수 적용
    float2 uv1 = input.uv * 2.5f + flowDir * phase1;

    // 두 노멀맵 샘플링 및 언팩 (-1 ~ 1)
    float3 n0 = NormalMap1.Sample(LinearSampler, uv0).rgb * 2.0f - 1.0f;
    float3 n1 = NormalMap2.Sample(LinearSampler, uv1).rgb * 2.0f - 1.0f;
    
    // 시간에 따라 두 페이즈를 부드럽게 블렌딩
    float blendFactor = abs((phase0 - 0.5f) * 2.0f);
    float3 localNormal = normalize(lerp(n0, n1, blendFactor));

    // 화면 공간 왜곡에 사용할 XY 오프셋
    float2 distortion = localNormal.xy * WATER_DISTORTION;

    // 왜곡된 UV 계산 (화면 밖을 벗어나지 않도록 saturate 안전장치)
    float2 reflectUV = saturate(screenUV + distortion);
    float2 refractUV = saturate(screenUV + distortion * 0.5f); // 굴절은 아티팩트 방지를 위해 왜곡을 절반으로 감쇄


    // 탄젠트 공간의 Z(Up)를 월드 공간의 Y(Up)로 매핑
    float3 worldNormal = normalize(float3(localNormal.x, localNormal.z, localNormal.y));

    float3 waterWorldPos = get_world_from_depth(screenUV, input.pos.z, VIEW_INV, PROJ_INV);
    float3 viewDir = normalize(CAMERA_POSITION - waterWorldPos);

    // 굴절 왜곡이 일어난 위치의 바닥(지형) 깊이를 샘플링
    float floorDepthRaw = SceneDepthMap.Sample(LinearSampler, refractUV).r;
    
    // Common.hlsli의 depth_to_meter 활용하여 미터 단위 선형 깊이 변환
    float floorDepthLinear = depth_to_meter(floorDepthRaw, PROJ);
    float waterDepthLinear = depth_to_meter(input.pos.z, PROJ);
    
    // 물의 실제 수직 깊이 계산
    float actualWaterDepth = max(floorDepthLinear - waterDepthLinear, 0.0f);

    // 만약 왜곡된 UV가 물보다 앞에 있는 물체(예: 캐릭터 상체 등)를 찔렀다면 오작동하므로 예외 처리
    if (floorDepthLinear < waterDepthLinear)
    {
        // 왜곡되지 않은 원본 깊이로 재시도하여 아티팩트 차단
        floorDepthRaw = SceneDepthMap.Sample(LinearSampler, screenUV).r;
        floorDepthLinear = depth_to_meter(floorDepthRaw, PROJ);
        actualWaterDepth = max(floorDepthLinear - waterDepthLinear, 0.0f);
        refractUV = screenUV; // 굴절 UV도 원본으로 롤백하여 테두리가 깨지는 현상 방지
    }

    // Beer's Law 적용: 깊어질수록 빛이 흡수되어 깊은 물 색상으로 전이
    float depthFactor = exp(-actualWaterDepth * WATER_DENSITY);
    float3 waterColor = lerp(WATER_COLOR_DEEP, WATER_COLOR_SHALLOW, depthFactor);

    float3 reflectColor = ReflectionMap.Sample(LinearSampler, reflectUV).rgb;
    float3 refractColor = RefractionMap.Sample(LinearSampler, refractUV).rgb;

    refractColor = lerp(waterColor, refractColor, depthFactor);


    float cosTheta = saturate(dot(viewDir, worldNormal));
    float fresnel = WATER_REFLECTIVITY + (1.0f - WATER_REFLECTIVITY) * pow(1.0f - cosTheta, 5.0f);

    // 프레넬 계수에 따라 반사(하늘/지형)와 굴절(물속)을 최종 블렌딩
    float3 finalColor = lerp(refractColor, reflectColor, fresnel);

    return float4(finalColor, 1.0f);
} // main