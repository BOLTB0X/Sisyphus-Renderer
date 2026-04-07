// Common.hlsli
#ifndef _COMMON_HLSLI_
#define _COMMON_HLSLI_

cbuffer FrameBuffer : register(b0)
{
    matrix cView;
    matrix cProjection;
    matrix cViewInv;
    matrix cProjInv;
    
    float3 cCameraPosition;
    float  fPadding1;
    
    float2 cScreenResolution;
    float  cTime;
    float  fPadding2;
}; // FrameBuffer

cbuffer DirectionalLightBuffer : register(b1)
{
    float3 cLightDirection;
    float  lPadding1;
    
    float4 cLightAmbient;
    float4 cLightDiffuse;
    
    float3 cLightLookAt;
    float  lPadding2;
    
    matrix cLightView;
    matrix cLightProj;
}; // DirectionalLightBuffer

#endif // _COMMON_HLSLI_