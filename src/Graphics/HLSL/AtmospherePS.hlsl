// AtmospherePS.hlsl
#include "AtmosphereParams.hlsli"

#define DEFAULT_LIGHT_DIR float3(0.0f, 1.0f, 0.0f)
#define ORIGIN            float3(0.0f, 0.0f, 0.0f)
#define MIN_DIST          float(1e-4f)
#define MAX_DIST          float(1e7f)

cbuffer LightBuffer : register(b1)
{
    float4 lDiffuseColor;
    float3 lLightDirection;
    float  lPadding;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 localPos : TEXCOORD0;
}; // PS_INPUT

float2 ray_sphere_intersect(float3 start, float3 dir, float radius)
{
    float a = dot(dir, dir);
    float b = 2.0 * dot(dir, start);
    float c = dot(start, start) - (radius * radius);
    float d = (b * b) - 4.0 * a * c;
    
    if (d < 0.0)
        return float2(1e5, -1e5);
    
    return float2(
        (-b - sqrt(d)) / (2.0 * a),
        (-b + sqrt(d)) / (2.0 * a)
    );
} // ray_sphere_intersect

float3 calculate_scattering(
    float3 start,
    float3 dir,
    float max_dist,
    float3 scene_color,
    float3 light_dir,
    float3 light_intensity,
    float3 planet_position,
    float planet_radius,
    float atmo_radius,
    float3 beta_ray,
    float beta_mie,
    float3 beta_absorption,
    float3 beta_ambient,
    float g,
    float height_ray,
    float height_mie,
    float height_absorption,
    float absorption_falloff,
    int steps_i,
    int steps_l
)
{
    start -= planet_position;
    
    float3 L = light_dir;
    float len2 = dot(L, L);
    if (len2 < 1e-8)
    {
        // fallback
        L = DEFAULT_LIGHT_DIR;
        len2 = 1.0f;
    }
    L = L * rsqrt(len2); // normalize

    float a = dot(dir, dir);
    float b = 2.0 * dot(dir, start);
    float c = dot(start, start) - (atmo_radius * atmo_radius);
    float d = b * b - 4.0 * a * c;

    if (d < 0.0)
        return scene_color;

    float2 ray_length = float2(
        max((-b - sqrt(d)) / (2.0 * a), 0.0),
        min((-b + sqrt(d)) / (2.0 * a), max_dist)
    );

    if (ray_length.x > ray_length.y)
        return scene_color;

    bool allow_mie = max_dist > ray_length.y;

    ray_length.y = min(ray_length.y, max_dist);
    ray_length.x = max(ray_length.x, 0.0);

    float step_size_i = (ray_length.y - ray_length.x) / steps_i;
    float ray_pos_i = ray_length.x + step_size_i * 0.5;

    float3 total_ray = 0;
    float3 total_mie = 0;
    float3 opt_i = 0;

    float2 scale_height = float2(height_ray, height_mie);

    float mu = dot(dir, L);
    float mumu = mu * mu;
    float gg = g * g;

    float phase_ray = 3.0 / (16.0 * 3.14159265) * (1.0 + mumu);
    float denom_phase = 1.0 + gg - 2.0 * mu * g;
    float phase_mie = allow_mie ?
        3.0 / (8.0 * 3.14159265) *
        ((1.0 - gg) * (mumu + 1.0)) /
        (pow(max(denom_phase, 1e-6), 1.5) * (2.0 + gg))
        : 0.0;

    for (int i = 0; i < steps_i; ++i)
    {
        float3 pos_i = start + dir * ray_pos_i;
        float height_i = length(pos_i) - planet_radius;

        float3 density;
        density.x = exp(-height_i / scale_height.x);
        density.y = exp(-height_i / scale_height.y);
        density.z = 0.0;
        
        float denom_abs = (height_absorption - height_i) / absorption_falloff;
        density.z = (1.0 / (denom_abs * denom_abs + 1.0)) * density.x;
        density *= step_size_i;

        opt_i += density;
        float aL = 1.0f;
        float bL = 2.0f * dot(L, pos_i);
        float cL = dot(pos_i, pos_i) - (atmo_radius * atmo_radius);
        float dL = bL * bL - 4.0 * aL * cL;

        float3 opt_l = 0;

        if (dL > 1e-6)
        {
            float step_size_l = (-bL + sqrt(dL)) / (2.0 * aL * steps_l);
            float ray_pos_l = step_size_l * 0.5;

            for (int l = 0; l < steps_l; ++l)
            {
                float3 pos_l = pos_i + L * ray_pos_l;

                float height_l = length(pos_l) - planet_radius;

                float3 density_l;
                density_l.x = exp(-height_l / scale_height.x);
                density_l.y = exp(-height_l / scale_height.y);
                density_l.z = 0;

                float denom2 = (height_absorption - height_l) / absorption_falloff;
                density_l.z = (1.0 / (denom2 * denom2 + 1.0)) * density_l.x;

                density_l *= step_size_l;

                opt_l += density_l;

                ray_pos_l += step_size_l;
            }
        }

        float3 attn =
            exp(-beta_ray * (opt_i.x + opt_l.x)
                - beta_mie * (opt_i.y + opt_l.y)
                - beta_absorption * (opt_i.z + opt_l.z));

        total_ray += density.x * attn;
        total_mie += density.y * attn;

        ray_pos_i += step_size_i;
    }

    float3 opacity = exp(-(beta_mie * opt_i.y
        + beta_ray * opt_i.x
        + beta_absorption * opt_i.z));

    return (phase_ray * beta_ray * total_ray +
        phase_mie * beta_mie * total_mie +
        opt_i.x * beta_ambient)
        * light_intensity
        + scene_color * opacity;
} // calculate_scattering

float3 calculate_ground_color(float3 ro, float3 rd, float2 planet_intersect)
{
    float3 final_ground_color = aGroundColor;
    
    // 히트 지점 계산
    float t_hit = max(planet_intersect.x, 0.0);
    float3 hit_pos = ro + rd * t_hit;
    float3 surface_normal = normalize(hit_pos - aPlanetCenter);
        
    // 그림자 및 라이팅 계산
    float3 L = -lLightDirection;
    float3 V = -rd;
    float dotNL = max(MIN_DIST, dot(surface_normal, L));
    float dotNV = max(MIN_DIST, dot(surface_normal, V));
    float shadow = dotNL / (dotNL + dotNV);
    
    final_ground_color *= shadow;
    
    float3 bent_normal = normalize(lerp(surface_normal, L, 0.6));

    // 지표면에서 하늘 방향으로의 간접광 산란
    float3 sky_ambient = calculate_scattering(
            hit_pos, bent_normal, 3.0 * aAtmoRadius,
            ORIGIN, L, float3(aIntensity, aIntensity, aIntensity),
            aPlanetCenter, aPlanetRadius, aAtmoRadius,
            aRayleighBeta, aMieBeta, aAbsorptionBeta, float3(aAmbientBeta, aAmbientBeta, aAmbientBeta),
            aG, aRayleighHeight, aMieHeight, aAbsorptionHeight, aAbsorptionFalloff,
            aGroundPrimarySteps, agroundLightSteps
    );

    final_ground_color += clamp(sky_ambient * aGroundColor, 0.0, 1.0);
    
    return final_ground_color;
} // calculate_ground_color

float3 sun_color(float3 rd, float3 light_dir, float intensity)
{
    float sunCos = dot(rd, light_dir);
    float sunDisk = smoothstep(0.998, 1.0, sunCos);
    return sunDisk * float3(1.0, 0.9, 0.8) * intensity * 50.0;
} // sun_color

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 rd = normalize(input.localPos); // 시선 방향
    float3 ro = cCameraPosition / 1000.0f; // 카메라 위치
    
    float2 planet_intersect = ray_sphere_intersect(ro - aPlanetCenter, rd, aPlanetRadius);
   
    float max_dist = MAX_DIST;
    float3 scene_color = dot(rd, lLightDirection) > 0.9998 ? 3.0 : 0.0;
    
    if (planet_intersect.y > 0.0)
    {
        max_dist = max(planet_intersect.x, 0.0);
        scene_color = calculate_ground_color(ro, rd, planet_intersect);
    } else
    {
        scene_color = sun_color(rd, -lLightDirection, aIntensity);
    }

    float3 col = calculate_scattering(
        ro, rd, max_dist,
        scene_color,
        -lLightDirection,
        float3(aIntensity, aIntensity, aIntensity),
        aPlanetCenter,
        aPlanetRadius,
        aAtmoRadius,
        aRayleighBeta,
        aMieBeta,
        aAbsorptionBeta,
        float3(aAmbientBeta, aAmbientBeta, aAmbientBeta),
        aG,
        aRayleighHeight,
        aMieHeight,
        aAbsorptionHeight,
        aAbsorptionFalloff,
        aPrimarySteps,
        aLightSteps
    );
    
    col = 1.0 - exp(-1.0 * col);
    
    // CPU 전달 및 레이 디버깅 
    //float3 debugColor = rd * 0.5f + 0.5f;
    //return float4(debugColor, 1.0f);
    //return float4(rd * 0.5f + 0.5f, 1.0f);
    return float4(col, 1);
} // main