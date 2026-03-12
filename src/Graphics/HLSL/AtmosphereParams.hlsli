// AtmosphereParams.hlsli
// https://www.shadertoy.com/view/wlBXWK
#ifndef ATMOSPHERE_PARAMS_HLSLI
#define ATMOSPHERE_PARAMS_HLSLI

cbuffer CameraBuffer : register(b2)
{
    float3 cCameraPosition; // 카메라 월드 위치
    float4x4 cViewInv;
    float4x4 cProjInv;
}; // CameraBuffer

cbuffer AtmosphereBuffer : register(b3)
{
    // [Row 1] 단순 그라데이션
    float4 aZenithColor;
    // [Row 2] 단순 그라데이션
    float4 aHorizonColor;
    // [Row 3] 행성 물리 데이터
    float3 aPlanetCenter;
    float  aPlanetRadius;
    // [Row 4] 대기권 물리 데이터
    float  aAtmoRadius;
    float3 aPadding;
    // [Row 5] 산란 계수 (Rayleigh)
    float3 aRayleighBeta;
    float  aMieBeta;
    // [Row 6] 흡수 및 주변광
    float3 aAbsorptionBeta;
    float  aAmbientBeta;
    // [Row 7] 고도 상수 (Density Falloff)
    float  aRayleighHeight;
    float  aMieHeight;
    float  aAbsorptionHeight;
    float  aAbsorptionFalloff;
    // [Row 8] Mie 위상 함수 및 샘플링 설정
    float  aG;
    int    aPrimarySteps;
    int    aLightSteps;
    float  aIntensity;
    // [Row 9] 지표면 색상
    float3 aGroundColor;
    float  aPadding2;
    // [Row 10] 지표면 레이마칭 설정
    int    aGroundPrimarySteps;
    int    agroundLightSteps;
    float  aPadding3;
}; // AtmosphereBuffer
#endif