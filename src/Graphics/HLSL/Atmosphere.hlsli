// Atmosphere.hlsli
// https://www.shadertoy.com/view/wlBXWK
// https://www.shadertoy.com/view/XslGRr
// https://www.shadertoy.com/view/Xttcz2
// https://www.shadertoy.com/view/MstBWs
// https://github.com/chihirobelmo/volumetric-cloud-for-directx11/blob/main/VolumetricCloud/shaders/CommonFunctions.hlsl
// https://github.com/fede-vaccaro/TerrainEngine-OpenGL/blob/master/shaders/volumetric_clouds.comp
#ifndef _ATMOSPHERE_HLSLI_
#define _ATMOSPHERE_HLSLI_

#include "Common.hlsli"

#define DEFAULT_LIGHT_DIR float3(0.0f, -1.0f, 0.0f)
#define ORIGIN            float3(0.0f, 0.0f, 0.0f)

float3 ray_apply_curvature(float3 rayPos, float3 rayStart, float earthRadius)
{
    float3 dir = normalize(rayPos - rayStart);
    float dist = length(rayPos - rayStart);
    float angle = dist / earthRadius;
    
    return rayStart + earthRadius * sin(angle) * dir
           - float3(0, earthRadius * (1.0 - cos(angle)), 0);
} // ray_apply_curvature

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

float2 atmosphere_earth_intersect(float3 ro, float3 rayDir, float atmos_radius_meter, float earth_radius_meter)
{
    float2 p = ray_sphere_intersect(float3(0, -ro.y + earth_radius_meter, 0), rayDir * float3(1, -1, 1), atmos_radius_meter);
    p.y = min(p.y, ray_sphere_intersect(float3(0, -ro.y + earth_radius_meter, 0), rayDir * float3(1, -1, 1), earth_radius_meter).x);
    return p;
} // atmosphere_earth_intersect

float2 ray_shell_intersection(float3 ro_meter, float3 rd, float earthRadius, float minHeight, float maxHeight)
{
    // 행성 중심을 원점으로 맞춤 (카메라 위치에 지구 반지름만큼 더함)
    float3 planetCenter = float3(0, -earthRadius, 0);
    float3 relativeRo = ro_meter - planetCenter;

    // 하단(MinHeight) 구체와 교차 확인
    float2 innerInter = ray_sphere_intersect(relativeRo, rd, earthRadius + minHeight);
    // 상단(MaxHeight) 구체와 교차 확인
    float2 outerInter = ray_sphere_intersect(relativeRo, rd, earthRadius + maxHeight);

    // 구름 레이어 진입점(tNear)과 진출점(tFar) 계산
    // 보통 카메라가 구름 아래에 있다면 outerInter.x가 진입점, outerInter.y가 진출점이 되거나
    // 하단 구체를 뚫고 올라가는 형태가 됌
    float tNear = max(0.0, outerInter.x);
    float tFar = outerInter.y;

    // 만약 카메라가 구름 아래 있다면, 하단 구체를 뚫고 나가는 지점부터 상단 구체까지가 구름 영역
    if (length(relativeRo) < earthRadius + minHeight)
    {
        tNear = max(tNear, innerInter.y);
    }

    return float2(tNear, tFar);
} // ray_shell_intersection

float3 calculate_atmosphere_scattering(
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

    float step_size_i = (ray_length.y - ray_length.x) / max(steps_i, 1);
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
    
    [loop]
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

float3 calculate_ground_scattering(
    float3 ro, float3 rd,
    float2 planet_intersect,
    float3 ground_color,
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
    float3 res = ground_color;
    
    // 히트 지점 계산
    float t_hit = max(planet_intersect.x, 0.0);
    float3 hit_pos = ro + rd * t_hit;
    float3 surface_normal = normalize(hit_pos - planet_position);
    
        
    // 그림자 및 라이팅 계산
    float3 L = -light_dir;
    float3 V = -rd;
    float dotNL = max(MIN_DIST, dot(surface_normal, L));
    float dotNV = max(MIN_DIST, dot(surface_normal, V));
    float shadow = dotNL / (dotNL + dotNV);
    
    res *= shadow;
    
    float3 bent_normal = normalize(lerp(surface_normal, L, 0.6));

    // 지표면에서 하늘 방향으로의 간접광 산란
    float3 sky_ambient = calculate_atmosphere_scattering(
            hit_pos, bent_normal, max_dist,
            scene_color, L, light_intensity,
            planet_position, planet_radius, atmo_radius,
            beta_ray, beta_mie, beta_absorption, beta_ambient,
            g, height_ray, height_mie, height_absorption, absorption_falloff,
            steps_i, steps_l);

    res += clamp(sky_ambient * ground_color, 0.0, 1.0);
    
    return res;
} // calculate_ground_scattering

float2 ray_sphere_Intersect_for_suncolor(float3 start, /* ray 의 시작 위치 */ float3 dir, /* ray 의 방향*/ float radius)
{
    // result.x > result.y인 경우 교차가 없음
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
} // ray_sphere_Intersect_for_suncolor

float3 calculate_sunlight_color(float3 sunDir, float earthRadius)
{
    sunDir.y *= -1.0; // Invert the Y-axis

    // Constants for atmospheric scattering
    const float3 rayleighCoeff = float3(0.0058, 0.0135, 0.0331); // 레일리 산란 계수(R, G, B)
    const float3 mieCoeff = float3(0.0030, 0.0030, 0.0030); // Mie 산란 계수(R, G, B)
    const float rayleighScaleDepth = 0.25; // 레일리 산란 스케일
    const float mieScaleDepth = 0.1; // Mie 산란 스케일

    // 태양 고도 각도
    float sunAltitude = asin(sunDir.y);
    float sunZenithAngle = max(0.0, 1.0 - sunDir.y); // 0: 태양이 머리 위에 있음, 1: 태양이 지평선에 있음

    // 천정 부근에서 0으로 나누는 것을 생략
    float safeSunDirY = max(abs(sunDir.y), 0.01);

    // 대기 질량의 근사치 (햇빛이 통과하는 대기의 양)
    float rayleighAirMass = exp(-safeSunDirY / rayleighScaleDepth) / (safeSunDirY + 0.15 * pow(93.885 - sunZenithAngle * 180.0 / 3.14159, -1.253));
    float mieAirMass = exp(-safeSunDirY / mieScaleDepth) / (safeSunDirY + 0.15 * pow(93.885 - sunZenithAngle * 180.0 / 3.14159, -1.253));

    // 산란에 의한 햇빛의 감쇠
    float3 rayleighAttenuation = exp(-rayleighCoeff * rayleighAirMass);
    float3 mieAttenuation = exp(-mieCoeff * mieAirMass);

    // 햇빛 색상 (산란 감쇠 적용)
    float3 sunlightColor = float3(1.0, 1.0, 1.0) * rayleighAttenuation * mieAttenuation;

    float3 pos = float3(0, 5000 + earthRadius, 0); // Ray origin
    float2 planet_intersect = ray_sphere_Intersect_for_suncolor(pos - /*earth position*/0.0, sunDir, earthRadius);
    float4 color = float4(sunlightColor, 1e12);
    // 광선이 행성에 닿으면 해당 광선의 최대 거리를 설정
    if (0.0 < planet_intersect.y)
    {
        color.w = max(planet_intersect.x, 0.0);
        
        // 픽셀이 있는 샘플 위치
        float3 sample_pos = pos + (sunDir * planet_intersect.x) - earthRadius;
        
        float3 surface_normal = normalize(sample_pos);
        color.xyz = float3(0.5, 0.5, 0.5);
        
        // 이 지점이 쉐도우에 가려지는지 여부,
        // lommel-seelinger  법칙에 따라 카메라 쪽으로 산란되는 빛의 양을 구함
        float3 N = surface_normal;
        float3 L = sunDir;
        float dotNL = max(1e-6, dot(N, L));
        float shadow = dotNL;
        
        // apply the shadow
        color.xyz *= shadow;
    }

    return color.xyz;
} // calculate_sunlight_color

#endif // _ATMOSPHERE_HLSLI_