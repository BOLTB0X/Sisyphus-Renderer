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
// https://patapom.com/topics/Revision2013/Revision%202013%20-%20Real-time%20Volumetric%20Rendering%20Course%20Notes.pdf
#ifndef _VOLUMETRICCLOUD_HLSLI_
#define _VOLUMETRICCLOUD_HLSLI_

#define CLOUD_MARCH_STEPS                   32
#define CLOUD_SELF_SHADOW_STEPS             8
#define CLOUDS_SHADOW_MARGE_STEP_SIZE       10.0f
#define CLOUDS_LAYER_SHADOW_MARGE_STEP_SIZE 4.0f
#define CLOUDS_SHADOW_MARGE_STEP_MULTIPLY   1.3f
#define CLOUDMAP_UV_OFFSET                  0.00005f
#define WORLEY_UV_OFFSET                    0.0016f
#define HEIGHT_BASED_FOG_B                  0.02f
#define HEIGHT_BASED_FOG_C                  0.05f
#define HENYEY_GREENSTEIN_SCALE             1.0f

float henyey_greenstein(float sundotrd, float g)
{
    float gg = g * g;
    return (1. - gg) / pow(1. + gg - 2. * g * sundotrd, 1.5);
} // henyey_greenstein

float linear_step(const float s, const float e, float v)
{
    return clamp((v - s) * (1.0f / (e - s)), 0.0f, 1.0f);
} // linear_step

float linear_step_org(const float e, float v)
{
    return min(v * (1.0f / e), 1.0f);
} // linear_step_org

float cloud_gradient(float norY)
{
    return linear_step(0.0f, 0.05f, norY) - linear_step(0.8f, 1.2f, norY);
} // cloud_gradient

float exponential_Integral(float z)
{
    return 0.5772156649015328606065 + log(1e-4 + abs(z)) + z * (1.0 + z * (0.25 + z * ((1.0 / 18.0) + z * ((1.0 / 96.0) + z *
(1.0 / 600.0))))); // For x!=0
} // exponential_Integral

float2 compute_ray_sphere_intersect(float3 ro, float3 rd, float3 sphereCenter, float radius)
{
    float3 L = ro - sphereCenter;
    float a = 1.0f; // rd는 normalize 되어있으므로 1
    float b = 2.0f * dot(rd, L);
    float c = dot(L, L) - (radius * radius);
    float d = b * b - 4.0f * a * c;

    if (d < 0.0f)
        return float2(-1.0f, -1.0f);

    float sqrt_d = sqrt(d);
    // x는 진입점(Near), y는 탈출점(Far)
    return float2((-b - sqrt_d) / 2.0f, (-b + sqrt_d) / 2.0f);
} // compute_ray_sphere_intersect

#endif // _VOLUMETRICCLOUD_HLSLI_