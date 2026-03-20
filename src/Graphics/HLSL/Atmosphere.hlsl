// https://www.shadertoy.com/view/wlBXWK
#define DEFAULT_LIGHT_DIR float3(0.0f, -1.0f, 0.0f)
#define ORIGIN            float3(0.0f, 0.0f, 0.0f)
#define MIN_DIST          float(1e-4f)
#define MAX_DIST          float(1e7f)

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

float3 sun_color(float3 rd, float3 light_dir, float intensity)
{
    float sunCos = dot(rd, light_dir);
    float sunDisk = smoothstep(0.998, 1.0, sunCos);
    return sunDisk * float3(1.0, 0.9, 0.8) * intensity * 50.0;
} // sun_color

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