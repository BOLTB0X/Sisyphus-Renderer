// VolumetricCloud.hlsli
// https://www.shadertoy.com/view/MdGfzh
// https://github.com/chihirobelmo/volumetric-cloud-for-directx11/blob/main/VolumetricCloud/shaders/RayMarch.hlsl
// https://github.com/fede-vaccaro/TerrainEngine-OpenGL/blob/master/shaders/volumetric_clouds.comp
// https://github.com/microsoft/DirectX-Graphics-Samples/tree/master
// https://wallisc.github.io/rendering/2020/05/02/Volumetric-Rendering-Part-1.html
// https://www.jpgrenier.org/clouds.html
// https://erk.fe.uni-lj.si/2025/papers/loboda%28real_time_volumetric%29.pdf
// https://forums.unrealengine.com/t/distorting-textures-with-flow-maps/68111
// https://www.guerrilla-games.com/media/News/Files/The-Real-time-Volumetric-Cloudscapes-of-Horizon-Zero-Dawn.pdf
#ifndef _VOLUMETRICCLOUD_HLSLI_
#define _VOLUMETRICCLOUD_HLSLI_

// ==========================================
// 상수 및 배열 정의
// =========================================
static const float3 windDirection = normalize(float3(0.5f, 0.0f, 0.1f));
static const float coverage_multiplier = 0.4f;
static const float crispiness = 0.4f;
static const float curliness = 0.1f;
static const float absorption = 0.0035f;
static const float densityFactor = 0.02f;

// 구름 타입별 그라데이션 (x: start_fade_in, y: end_fade_in, z: start_fade_out, w: end_fade_out)
static const float4 STRATUS_GRADIENT = float4(0.02f, 0.05f, 0.09f, 0.11f);
static const float4 STRATOCUMULUS_GRADIENT = float4(0.02f, 0.2f, 0.48f, 0.625f);
static const float4 CUMULUS_GRADIENT = float4(0.01f, 0.0625f, 0.78f, 1.0f);

// 4x4 디더링용 Bayer Filter (HLSL 배열 문법)
static const float bayerFilter[16] =
{
    0.0f / 16.0f, 8.0f / 16.0f, 2.0f / 16.0f, 10.0f / 16.0f,
   12.0f / 16.0f, 4.0f / 16.0f, 14.0f / 16.0f, 6.0f / 16.0f,
    3.0f / 16.0f, 11.0f / 16.0f, 1.0f / 16.0f, 9.0f / 16.0f,
   15.0f / 16.0f, 7.0f / 16.0f, 13.0f / 16.0f, 5.0f / 16.0f
};

// 빛 마칭용 랜덤 방향 벡터 (Cone Sampling)
static const float3 noiseKernel[6] =
{
    float3(0.3805f, 0.9245f, -0.0211f), float3(-0.5062f, -0.0359f, -0.8616f),
    float3(-0.3250f, -0.9455f, 0.0142f), float3(0.0902f, -0.2737f, 0.9575f),
    float3(0.2812f, 0.4244f, -0.8606f), float3(-0.4172f, 0.3553f, 0.8365f)
};
float henyeygreenstein(float sundotrd, float g)
{
    float gg = g * g;
    return (1.0f - gg) / pow(1.0f + gg - 2.0f * g * sundotrd, 1.5f);
} // henyeygreenstein

float linear_step(const float s, const float e, float v)
{
    return clamp((v - s) * (1.0f / (e - s)), 0.0f, 1.0f);
} // linear_step

float linear_step0(const float e, float v)
{
    return min(v * (1.0f / e), 1.0f);
} // linear_step0

float cloud_gradient(float norY)
{
    return linear_step(0.0f, 0.05f, norY) - linear_step(0.8f, 1.2f, norY);
} // cloud_gradient

float3 get_sun(float3 sunDir , const float3 d, float powExp)
{
    float sun = clamp(dot(sunDir, d), 0.0, 1.0);
    return 0.8 * float3(1.0, .6, 0.1) * pow(sun, powExp);
} // get_sun

float random2D(in float3 st, float time)
{
    return frac(sin(time * dot(st.xyz, float3(12.9898f, 78.233f, 57.152f))) * 43758.5453123f);
} // random2D

float get_height_fraction(float3 p, float3 center, float innerR, float outerR)
{
    return saturate((length(p - center) - innerR) / (outerR - innerR));
} // get_height_fraction

bool ray_sphere_intersection_dual(float3 ro, float3 rd, float3 center, float radius, out float tNear, out float tFar)
{
    float3 oc = ro - center;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - radius * radius;
    float h = b * b - c;
    
    if (h < 0.0f)
        return false;
    
    h = sqrt(h);
    tNear = -b - h;
    tFar = -b + h;
    return true;
} // ray_sphere_intersection_dual

// 더 직관적이고 안전한 교차 판정 함수
float2 get_ray_sphere_intersect(float3 ro, float3 rd, float3 sphereCenter, float radius)
{
    float3 L = ro - sphereCenter;
    float a = 1.0f; // rd는 normalize 되어있으므로 1
    float b = 2.0f * dot(rd, L);
    float c = dot(L, L) - (radius * radius);
    float d = b * b - 4.0f * a * c;

    if (d < 0.0f)
        return float2(-1.0f, -1.0f); // 교차하지 않음

    float sqrt_d = sqrt(d);
    // x는 진입점(Near), y는 탈출점(Far)
    return float2((-b - sqrt_d) / 2.0f, (-b + sqrt_d) / 2.0f);
}

float powder(float d)
{
    return (1. - exp(-2. * d));
} // powder

#endif // _VOLUMETRICCLOUD_HLSLI_