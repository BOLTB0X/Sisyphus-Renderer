// SkyBoxPS.hlsl
// https://www.shadertoy.com/view/wlBXWK
#include "Common.hlsli"
#include "Atmosphere.hlsli"
#include "Remap.hlsli"

SamplerState LinearWrapSampler : register(s0);
Texture2D SceneDepth : register(t1);
Texture2D SkyLUT : register(t2);

struct PS_IN
{
    float4 position : SV_POSITION;
    float3 localPos : TEXCOORD0;
}; // PS_INPUT

struct PS_OUT
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
}; // PS_OUT

PS_OUT main(PS_IN input) : SV_TARGET
{  
    PS_OUT output;
    float3 rd = normalize(input.localPos);
    
    float3 ro = CAMERA_POSITION; // 카메라 위치
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
    
    //return float4(skyColor, 1.0f);   
    output.color = float4(skyColor, 1.0f);
    output.normal = float4(0.0f, 0.0f, 0.0f, 1.0f);
    return output;
} // main