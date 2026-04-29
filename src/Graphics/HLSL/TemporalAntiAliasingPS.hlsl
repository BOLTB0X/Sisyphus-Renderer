// TemporalAntiAliasingPS.hlsl
// https://www.shadertoy.com/view/4dSBDt
// https://www.moddb.com/mods/stalker-anomaly/addons/temporal-anti-aliasing
// https://github.com/fede-vaccaro/TerrainEngine-OpenGL/blob/master/shaders/copyFrame.frag
#include "Common.hlsli"
#include "PostProcess.hlsli"

cbuffer TAABuffer : register(b2)
{
    // Row 1 ~ 2
    matrix tPrevViewProj;
    // Row 3
    float  tBlendFactor;
    float3 tPadding1;
    // Row 4
    float2 tTexelSize;
    float2 tPadding2;
}; // TAABuffer

SamplerState LinearSampler : register(s0);
Texture2D    CurrentTex : register(t0);
Texture2D    HistoryTex : register(t1);
Texture2D    DepthTex : register(t2);

#define PREV_VIEW_PROJ     tPrevViewProj
#define BLEND_FACTOR       tBlendFactor
#define TEXEL_SIZE         tTexelSize
#define G_COLOR_BOX_SIGMA  g_color_box_sigma

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 current = RGB_to_YCoCg(CurrentTex.Sample(LinearSampler, input.uv).rgb);
    
    // Reprojection (모션 벡터 계산)
    // 1) 현재 픽셀의 깊이(Depth) 값 샘플링
    float depth = DepthTex.Sample(LinearSampler, input.uv).r;
    
    // 2) Common.hlsli의 함수를 이용해 현재 픽셀의 월드 좌표(World Position) 복원
    float3 worldPos = get_world_from_depth(input.uv, depth, VIEW_INV, PROJ_INV);
    
    // 3) 복원된 월드 좌표를 '이전 프레임'의 카메라 행렬로 곱하여 이전 프레임의 화면 공간으로 변환
    float4 prevClipPos = mul(float4(worldPos, 1.0f), PREV_VIEW_PROJ);
    prevClipPos /= prevClipPos.w; // Perspective Divide
    
    // 4) Clip Space (-1 ~ 1) 좌표를 UV Space (0 ~ 1)로 변환
    float2 prevUV = prevClipPos.xy * 0.5f + 0.5f;
    prevUV.y = 1.0f - prevUV.y;
    // ------------------------------------------

    // History 샘플링
    float3 history = RGB_to_YCoCg(HistoryTex.Sample(LinearSampler, prevUV).rgb);

    // 화면 밖으로 나간 UV에 대한 처리 (가장자리 깜빡임 방지)
    if (prevUV.x < 0.0f || prevUV.x > 1.0f || prevUV.y < 0.0f || prevUV.y > 1.0f)
    {
        // 이전 프레임에 화면에 없던 정보이므로 History를 무시하고 현재 프레임만 사용
        return float4(YCoCg_to_RGB(current), 1.0f);
    }

    // Variance Clipping (고스트 현상 제거)
    float3 colorAvg = current;
    float3 colorVar = current * current;

    [unroll]
    for (int i = 0; i < 8; i++)
    {
        float2 offsetUV = input.uv + float2(offsets_TAA[i]) * TEXEL_SIZE;
        float3 fetch = RGB_to_YCoCg(CurrentTex.Sample(LinearSampler, offsetUV).rgb);
        colorAvg += fetch;
        colorVar += fetch * fetch;
    }
    colorAvg /= 9.0f;
    colorVar /= 9.0f;

    float3 sigma = sqrt(max(float3(0, 0, 0), colorVar - colorAvg * colorAvg));
    float3 colorMin = colorAvg - G_COLOR_BOX_SIGMA * sigma;
    float3 colorMax = colorAvg + G_COLOR_BOX_SIGMA * sigma;

    // History 색상이 현재 프레임의 주변 색상 범위를 벗어나면 강제로 잘라냄(Clamp)
    history = clamp(history, colorMin, colorMax);

    // 최종 블렌딩
    float3 result = YCoCg_to_RGB(lerp(current, history, BLEND_FACTOR));
    return float4(result, 1.0f);
} // main