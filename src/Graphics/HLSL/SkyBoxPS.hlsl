// SkyBoxPS.hlsl
// https://www.shadertoy.com/view/wlBXWK
// https://github.com/chihirobelmo/volumetric-cloud-for-directx11/blob/main/VolumetricCloud/shaders/RayMarch.hlsl
// https://github.com/fede-vaccaro/TerrainEngine-OpenGL/blob/master/shaders/volumetric_clouds.comp
// https://wallisc.github.io/rendering/2020/05/02/Volumetric-Rendering-Part-1.html
// https://www.jpgrenier.org/clouds.html
// https://erk.fe.uni-lj.si/2025/papers/loboda%28real_time_volumetric%29.pdf
// https://forums.unrealengine.com/t/distorting-textures-with-flow-maps/68111
// https://forums.odforce.net/topic/23724-flow-map-from-fluid-velocity-for-game-application/
// https://www.guerrilla-games.com/read/nubis-authoring-real-time-volumetric-cloudscapes-with-the-decima-engine
// https://www.guerrilla-games.com/media/News/Files/The-Real-time-Volumetric-Cloudscapes-of-Horizon-Zero-Dawn.pdf
// https://github.com/microsoft/DirectX-Graphics-Samples/tree/master
#include "Common.hlsli"
#include "Atmosphere.hlsli"
#include "Remap.hlsli"
#include "Debugs.hlsli"
#define  NM_TO_M 1852
#define  FT_TO_M 0.3048
#define  LIGHT_MARCH_SIZE 400.0f
#define  MAX_LENGTH 422440.0f

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 localPos : TEXCOORD0;
}; // PS_INPUT

cbuffer CloudBoxBuffer : register(b4)
{
    float4 cBoxCenter;
    float4 cBoxSize;
    float cEarthRadius; // 6371000.0f
    float cCloudMinHeight;
    float cCloudMaxHeight;
    float cPadding;
}; // CloudBoxBuffer

SamplerState LinearWrapSampler : register(s0);
Texture2D    SceneDepthTexture : register(t1);
Texture2D    SkyVolumeLUT : register(t2);

float BeerLambert(float density, float stepSize)
{
    return exp(-density * stepSize);
} // BeerLambert

float Energy(float density, float stepSize, float HG)
{
    return exp(-density * stepSize); // * (1.0 - exp( - 2.0 * density * stepSize )) * HG;
} // Energy

float Powder(float density, float stepSize)
{
    return 1.0 - exp(-2.0 * density * stepSize);
} // Powder

float HenyeyGreenstein(float cos_angle, float eccentricity)
{
    return ((1.0 - eccentricity * eccentricity) / pow((1.0 + eccentricity * eccentricity - 2.0 * eccentricity * cos_angle), 3.0 / 2.0)) / 4.0 * 3.1415;
} // HenyeyGreenstein

float3 RandomDirection(float3 seed)
{
    float phi = 2.0 * 3.14159 * frac(sin(dot(seed.xy, float2(12.9898, 78.233))) * 43758.5453);
    float costheta = 2.0 * frac(cos(dot(seed.xy, float2(23.14069, 90.233))) * 12345.6789) - 1.0;
    float sintheta = sqrt(1.0 - costheta * costheta);
    return float3(sintheta * cos(phi), sintheta * sin(phi), costheta);
} // RandomDirection

float4 main(PS_INPUT input) : SV_TARGET
{  
    float3 rd = normalize(input.localPos); // 시선 방향
    float3 ro = cCameraPosition / KM; // 카메라 위치
    float2 uv = input.position.xy / float2(cScreenResolution.x, cScreenResolution.y);
    float3 uvw = float3(uv, 1.0f);
    
    float2 skyUV = get_spherical_uv(rd);
    float3 skyColor = SkyVolumeLUT.Sample(LinearWrapSampler, skyUV).rgb;

    float2 screenUV = input.position.xy / cScreenResolution;
    float sceneDepth = SceneDepthTexture.Load(int3(input.position.xy, 0)).r;
    
    if (sceneDepth < 1.0f)
    {

    }
    skyColor = 1.0 - exp(-1.0 * skyColor);
    return float4(skyColor, 1.0f);   
} // main