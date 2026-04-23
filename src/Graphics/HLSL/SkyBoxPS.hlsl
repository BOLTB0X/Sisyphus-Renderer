// SkyBoxPS.hlsl
// https://www.shadertoy.com/view/wlBXWK
#include "Common.hlsli"
#include "Atmosphere.hlsli"
#include "Remap.hlsli"

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 localPos : TEXCOORD0;
}; // PS_INPUT

SamplerState LinearWrapSampler : register(s0);
Texture2D    SceneDepth : register(t1);
Texture2D    SkyLUT : register(t2);

float4 main(PS_INPUT input) : SV_TARGET
{  
    float3 rd = normalize(input.localPos);
    float3 ro = CAMERA_POSITION / KM; // 카메라 위치
    float2 uv = input.position.xy / float2(SCREEN_RESOLUTION.x, SCREEN_RESOLUTION.y);
    float3 uvw = float3(uv, 1.0f);
    
    float2 skyUV = get_spherical_uv(rd);
    float3 skyColor = SkyLUT.Sample(LinearWrapSampler, skyUV).rgb;

    float2 screenUV = input.position.xy / SCREEN_RESOLUTION;
    float sceneDepth = SceneDepth.Load(int3(input.position.xy, 0)).r;
    
    if (sceneDepth < 1.0f)
    {

    }
    skyColor = 1.0 - exp(-1.0 * skyColor);
    return float4(skyColor, 1.0f);   
} // main