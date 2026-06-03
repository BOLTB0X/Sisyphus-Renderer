// ShadowMap.hlsli
// 참고: https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
//       https://mstone8370.tistory.com/25
//       https://www.rastertek.com/dx11win10tut41.html
#ifndef SHADOWMAP_HLSLI
#define SHADOWMAP_HLSLI

static const float2 poisson_disk[16] =
{
    float2(-0.94201624, -0.39906216), float2(0.94558609, -0.76890725),
    float2(-0.094184101, -0.92938870), float2(0.34495938, 0.29387760),
    float2(-0.91588581, 0.45771432), float2(-0.81544232, -0.87912464),
    float2(-0.38190847, 0.37932123), float2(0.97351111, 0.45599420),
    float2(0.53742981, -0.47373420), float2(0.23497881, -0.96788225),
    float2(-0.82544401, -0.12328243), float2(0.71355652, 0.17178331),
    float2(0.39912361, -0.27312111), float2(0.99955401, -0.61206121),
    float2(-0.57321611, 0.77884112), float2(-0.31211321, 0.55621321)
}; // poisson_disk

// PCF
float calculate_pcf_shadow(SamplerState shadowSampler, Texture2D shadowMap, float4 lightClipPos, float bias)
{
    float2 projectTexCoord;
    projectTexCoord.x = lightClipPos.x / lightClipPos.w / 2.0f + 0.5f;
    projectTexCoord.y = -lightClipPos.y / lightClipPos.w / 2.0f + 0.5f;

    float shadowFactor = 1.0f; // 기본은 빛을 받는 상태

    if (saturate(projectTexCoord.x) == projectTexCoord.x && saturate(projectTexCoord.y) == projectTexCoord.y)
    {
        float shadowMapDepth = shadowMap.Sample(shadowSampler, projectTexCoord).r;
        float currentDepth = lightClipPos.z / lightClipPos.w;

        if (currentDepth > shadowMapDepth + bias)
        {
            shadowFactor = 0.5f;
        }
    }
    return shadowFactor;
} // calculate_pcf_shadow

// Poisson Sampling
float calculate_poisson_shadow(SamplerComparisonState shadowSampler, Texture2D shadowMap, float4 lightClipPos, float2 shadowMapSize, float spread, float bias)
{
    float2 uv = lightClipPos.xy / lightClipPos.w * float2(0.5f, -0.5f) + 0.5f;
    float currentDepth = lightClipPos.z / lightClipPos.w;
    
    if (currentDepth > 1.0f)
        return 1.0f;

    // 빛 범위 밖 처리
    if (saturate(uv.x) != uv.x || saturate(uv.y) != uv.y)
    {
        return 1.0f;
    }
    
    if (uv.x < 0.0f || uv.x > 1.0f ||
        uv.y < 0.0f || uv.y > 1.0f ||
        currentDepth < 0.0f || currentDepth > 1.0f)
    {
        return 1.0f;
    }

    float shadowFactor = 0.0f;
    float2 texelSize = 1.0f / shadowMapSize;
    
    // 16번 샘플링 루프
    for (int i = 0; i < 16; i++)
    {
        float2 sampleUV = uv + poisson_disk[i] * texelSize * spread;
        shadowFactor += shadowMap.SampleCmpLevelZero(shadowSampler, sampleUV, currentDepth - bias);
    }

    return shadowFactor / 16.0f;
} // calculate_poisson_shadow

#endif