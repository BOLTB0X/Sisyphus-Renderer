// VolumetricCloudCS.hlsl
// https://www.shadertoy.com/view/MdGfzh
// https://www.shadertoy.com/view/4dSBDt
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
// https://patapom.com/topics/Revision2013/Revision%202013%20-%20Real-time%20Volumetric%20Rendering%20Course%20Notes.pdf
// https://iquilezles.org/articles/fog/
#include "Common.hlsli"
#include "Volumetric.hlsli"
#include "Remap.hlsli"
#include "Noise.hlsli"

RWTexture2D<float4> OutTexture : register(u0);
RWTexture2D<float>  OutTransmittance : register(u1);
SamplerState        LinearWrapSampler : register(s0);
SamplerState        PointClampSampler : register(s1);
Texture2D<float>    SceneDepth : register(t1);
Texture2D           CloudMap : register(t2);
Texture3D           WorleyNoise : register(t3);
Texture2D           BlueNoise : register(t4);
Texture2D           SkyLUT : register(t5);

cbuffer VolumetricCloudBuffer : register(b2)
{
    // Row 1: 행성 기본 정보
    float3 vPlanetCenter;
    float  vPlanetRadius;
    // Row 2: 구름 영역
    float  vCloudBottom;
    float  vCloudTop;
    float2 vPadding1;
    // Row 3: 구름 형상 제어
    float  vCloudCoverage;
    float  vCloudBaseScale;
    float  vCloudDetailScale;
    float  vPadding2;
    // Row 4 : 구름 밀도 및 가장자리 부드러움
    float  vCloudDensity;
    float  vBaseEdgeSoftness;
    float  vBottomSoftness;
    float  vDetailStrength;
    // Row 5: 빛의 산란
    float  vForwardScatteringG;
    float  vBackwardScatteringG;
    float  vScatteringLerp;
    float  vMinTransmittance;
    // Row 6: 구름 색상 (Ambient)
    float3 vAmbientBottom;
    float  vPadding3;
    // Row 7: 구름 색상 (Ambient)
    float3 vAmbientTop;
    float  vPadding4;
    // Row 8: 구름 색상 (Ambient)
    float3 vSunsetAmbientBottom;
    float  vPadding5;
    // Row 9: 구름 색상 (Ambient)
    float3 vSunsetAmbientTop;
    float  vPadding6;
    // Row 10: 구름 색상 (Ambient)
    float3 vNightAmbientBottom;
    float  vPadding7;
    // Row 11: 구름 색상 (Ambient)
    float3 vNightAmbientTop;
    float  vPadding8;
    // Row 12: 바람
    float2 vWindDirection;
    float  vWindSpeed;
    float  vWindScale;
    // Row 13:
    float  vHgScale;
    float  vPowderFactor;
    float  vLightingScale;
    float  vHorizenFadeScale;
}; // VolumetricCloudBuffer

#define EARTH_RADIUS                 vPlanetRadius
#define CLOUDS_BOTTOM                vCloudBottom
#define CLOUDS_TOP                   vCloudTop
#define CLOUDS_COVERAGE              vCloudCoverage
#define CLOUDS_BASE_SCALE            vCloudBaseScale
#define CLOUDS_DETAIL_SCALE          vCloudDetailScale

#define CLOUDS_DENSITY               vCloudDensity
#define CLOUDS_BASE_EDGE_SOFTNESS    vBaseEdgeSoftness
#define CLOUDS_BOTTOM_SOFTNESS       vBottomSoftness
#define CLOUDS_DETAIL_STRENGTH       vDetailStrength

#define CLOUDS_FORWARD_SCATTERING_G  vForwardScatteringG
#define CLOUDS_BACKWARD_SCATTERING_G vBackwardScatteringG
#define CLOUDS_SCATTERING_LERP       vScatteringLerp
#define CLOUDS_MIN_TRANSMITTANCE     vMinTransmittance

#define CLOUDS_AMBIENT_COLOR_BOTTOM       vAmbientBottom
#define CLOUDS_AMBIENT_COLOR_TOP          vAmbientTop
#define CLOUD_SUNSET_AMBIENT_COLOR_BOTTOM vSunsetAmbientBottom
#define CLOUD_SUNSET_AMBIENT_COLOR_TOP    vSunsetAmbientTop
#define CLOUD_NIGHT_AMBIENT_COLOR_BOTTOM  vNightAmbientBottom
#define CLOUD_NIGHT_AMBIENT_COLOR_TOP     vNightAmbientTop

#define WIND_DIRECTION               vWindDirection
#define WIND_SPEED                   vWindSpeed
#define WIND_SCALE                   vWindScale

#define HENYEY_GREENSTEIN_SCALE      vHgScale
#define POWDER_FACTOR                vPowderFactor
#define LIGHTING_SCALE               vLightingScale
#define HORIZON_FADE_SCALE           vHorizenFadeScale

#define SPHERE_INNER_RADIUS          (EARTH_RADIUS + CLOUDS_BOTTOM)
#define SPHERE_OUTER_RADIUS          (EARTH_RADIUS + CLOUDS_TOP)

float GetCloudMapBase(float3 p, float norY, float dist = 0.0f)
{
    static const float CLOUDMAP_UV_OFFSET = 0.00005f;
    static const float MIPMAP_OFFSET = 50000.0f;
    
    float2 windOffset = WIND_DIRECTION * TIME * WIND_SPEED * WIND_SCALE;
    float3 pos = (p + float3(windOffset.x, 0, windOffset.y)) * (CLOUDMAP_UV_OFFSET * CLOUDS_BASE_SCALE);
    float mipLevel = clamp(log2(max(dist, 1.0f) / MIPMAP_OFFSET), 0.0f, 6.0f);
    float3 cloud = CloudMap.SampleLevel(LinearWrapSampler, pos.xz, mipLevel).rgb;
    
    float n = norY * norY;
    n *= cloud.b; // Cloud Type (B채널)
    n += pow(saturate(1.0f - norY), 16.0f);
    
    // R: 기본 밀도, G: 디테일 가이드
    return remap(cloud.r - n, cloud.g, 1.0f);
} // GetCloudMapBase

float GetWorleyNoiseMip(float3 pos, float dist)
{
    static const float MIPMAP_OFFSET = 50000.0f;
    static const float WORLEY_UV_OFFSET = 0.0016f;
    static const float AMPLITUDE_FACTOR = 0.707f;
    static const float FREQUENCY_FACTOR = 2.5789f;
    
    // 거리 기반 밉맵 레벨 계산
    float mipLevel = clamp(log2(dist / MIPMAP_OFFSET), 0.0f, 4.0f);
    
    float amplitude = 1.0f;
    float value = 0.0f;
    float totalAmplitude = 0.0f;
    float2 detailWindOffset = WIND_DIRECTION * TIME * WIND_SPEED * WIND_SCALE;
    float3 baseUVW = abs(pos) * (WORLEY_UV_OFFSET * CLOUDS_BASE_SCALE * CLOUDS_DETAIL_SCALE);
    float3 windOffset3D = float3(detailWindOffset.x, 0, detailWindOffset.y)
                          * (WORLEY_UV_OFFSET * CLOUDS_BASE_SCALE * CLOUDS_DETAIL_SCALE);
    
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        // mipLevel을 옥타브마다 올려서
        // 고주파 디테일을 점진적으로 제거
        float octaveMip = mipLevel + float(i) * 0.5f;
        value += amplitude * WorleyNoise.SampleLevel(LinearWrapSampler, baseUVW + windOffset3D, octaveMip).r;
        totalAmplitude += amplitude;
        amplitude *= AMPLITUDE_FACTOR;
        baseUVW *= FREQUENCY_FACTOR;
    }
    
    return value / totalAmplitude;
} // SampleOctaveNoise

float GetBlueNoise(uint2 pixelPos)
{
    float2 jitterUV = (float2(pixelPos) + float2(frac(TIME) * 123.4f, frac(TIME) * 567.8f)) / 128.0f;
    return BlueNoise.SampleLevel(LinearWrapSampler, jitterUV, 0).r;
} // GetBlueNoise

float3 GetSkyColor(float3 rd)
{
    float theta = atan2(rd.x, rd.z); // -PI ~ PI
    float phi = asin(rd.y); // -PI/2 ~ PI/2
    
    float2 uv;
    uv.x = (theta / (2.0f * PI)) + 0.5f;
    uv.y = 0.5f - (phi / PI);
    
    float3 skyColor = SkyLUT.SampleLevel(LinearWrapSampler, uv, 0).rgb;
    skyColor = 1.0f - exp(-1.0f * skyColor);
    return skyColor;
} // GetSkyColor

float ComputeCloudDensity(float3 pos, float norY, float dist = 0.0f, bool isShadow = false)
{
    float3 ps = pos;
    float m = GetCloudMapBase(ps, norY, dist);
    m *= cloud_gradient(norY);
    
    float dstrength = smoothstep(1.0f, 0.5f, m);
    
    if (!isShadow)
    {
        float detail = GetWorleyNoiseMip(pos, dist);
        m = remap_clamp(m, detail * dstrength * CLOUDS_DETAIL_STRENGTH, 1.0f, 0.0f, 1.0f);
    }

    // Coverage 및 Softness 보정
    m = smoothstep(0.0f, CLOUDS_BASE_EDGE_SOFTNESS, m + (CLOUDS_COVERAGE - 1.0f));
    m *= linear_step_org(CLOUDS_BOTTOM_SOFTNESS, norY);

    // 밀도 및 거리 감쇄
    return clamp(m * CLOUDS_DENSITY * density_distance_attenuation(pos.x), 0.0f, 1.0f);
} // ComputeCloudDensity

float RaymarchShadow(float3 from, float sundotrd, float3 sphereCenter,
                       float dC, float norY)
{
    int nbSampleLight = CLOUD_SELF_SHADOW_STEPS; // 4~6
    float zMaxl = 600.0f; // 빛 방향 샘플링 거리
    float stepL = zMaxl / float(nbSampleLight);

    float lighRayDen = 0.0f;

    float3 rd = LIGHT_DIRECTION;
    from += rd * stepL * jittering(from);

    for (int j = 0; j < nbSampleLight; j++)
    {
        float3 pos = from + rd * float(j) * stepL;
        float sampleNorY = clamp(
            (length(pos - sphereCenter) - (EARTH_RADIUS + CLOUDS_BOTTOM))
            / (CLOUDS_TOP - CLOUDS_BOTTOM), 0.0f, 1.0f);
        

        if (sampleNorY > 1.0f)
            break;

        lighRayDen += ComputeCloudDensity(pos, sampleNorY, lighRayDen, true);
    }
    
    // Beer's Law
    float mu = sundotrd;
    float scatterAmount = lerp(0.008f, 1.0f, smoothstep(0.96f, 0.0f, mu));
    float beersLaw = exp(-stepL * lighRayDen)
                        + 0.5f * scatterAmount * exp(-0.1f * stepL * lighRayDen)
                        + scatterAmount * 0.4f * exp(-0.02f * stepL * lighRayDen);

    // depth_probability
    float depth_prob = lerp(
        0.05f + 1.5f * pow(min(1.0f, dC * 8.5f), 0.3f + 5.5f * norY),
        1.0f,
        clamp(lighRayDen * 0.4f, 0.0f, 1.0f));

    float phaseFunction = lerp(henyey_greenstein(sundotrd, CLOUDS_FORWARD_SCATTERING_G),
                               henyey_greenstein(sundotrd, CLOUDS_BACKWARD_SCATTERING_G), CLOUDS_SCATTERING_LERP) * HENYEY_GREENSTEIN_SCALE;
    return beersLaw * phaseFunction * depth_prob;
} // RaymarchShadow

float4 RaymarchClouds(float3 ro, float3 rd, inout float sceneDist, uint2 pixelPos)
{
    if (rd.y < 0.0f)
        return float4(0, 0, 0, 1);

    float3 sphereCenter = float3(ro.x, -EARTH_RADIUS, ro.z);

    // 교차점 계산
    float2 inner = compute_ray_sphere_intersect(ro, rd, sphereCenter, SPHERE_INNER_RADIUS);
    float2 outer = compute_ray_sphere_intersect(ro, rd, sphereCenter, SPHERE_OUTER_RADIUS);
    
    if (outer.y < 0.0f)
    {
        return float4(0, 0, 0, 1);
    }
    float start = max(inner.y, outer.x); // 구름층 진입
    float end = outer.y; // 구름층 탈출
    
    end = min(end, sceneDist); // 렌더링된 다른 물체 앞까지만 마칭

    if (start >= end)
    {
        return float4(0, 0, 0, 1);
    }
    
    end = min(end, sceneDist);
    
    float dayFactor = saturate(-LIGHT_DIRECTION.y);
    float nightFactor = saturate(LIGHT_DIRECTION.y);

    float3 currentAmbientTop = lerp(CLOUD_SUNSET_AMBIENT_COLOR_BOTTOM, CLOUDS_AMBIENT_COLOR_TOP, dayFactor);
    currentAmbientTop = lerp(currentAmbientTop, CLOUD_NIGHT_AMBIENT_COLOR_TOP, nightFactor);

    float3 currentAmbientBottom = lerp(CLOUD_SUNSET_AMBIENT_COLOR_BOTTOM, CLOUDS_AMBIENT_COLOR_BOTTOM, dayFactor);
    currentAmbientBottom = lerp(currentAmbientBottom, CLOUD_NIGHT_AMBIENT_COLOR_BOTTOM, nightFactor);
   
    float d = start;
    float dD = (end - start) / (float) CLOUD_MARCH_STEPS;

    // 지터링
    float noise = GetBlueNoise(pixelPos);
    d -= dD * noise;

    float sundotrd = dot(rd, -LIGHT_DIRECTION);
    float transmittance = 1.0;
    float3 scatteredLight = float3(0.0, 0.0, 0.0);
    sceneDist = EARTH_RADIUS;

    for (int s = 0; s < CLOUD_MARCH_STEPS; s++)
    {
        float3 p = ro + rd * d;
        float norY = clamp((length(p - sphereCenter)
                        - (EARTH_RADIUS + CLOUDS_TOP)) / (CLOUDS_TOP - CLOUDS_BOTTOM), 0.0f, 1.0f);
        float alpha = ComputeCloudDensity(p, norY, d);

        if (alpha > 0.0f)
        {
            // 조명 적분
            float shadow = RaymarchShadow(p, sundotrd, sphereCenter, alpha, norY);
            float extCoeff = max(alpha * 0.1f, 0.001f);
            float3 ambientLight = compute_ambient_color(p, CLOUDS_TOP, CLOUDS_BOTTOM, extCoeff,
                            currentAmbientTop, currentAmbientBottom);
            float heightBright = compute_height_brightness(norY);
            float sunDimFactor = lerp(0.3f, 1.2f, dayFactor);
            float powder = 1.0f - exp(-alpha * 2.0f);
            float powderFactor = lerp(1.0f, powder, POWDER_FACTOR);
            float ms = compute_multiple_scattering(alpha, dD);
            
            float3 lighting = shadow + ms * LIGHTING_SCALE;
            float3 S = (ambientLight + get_dynamic_light_color(LIGHT_DIRECTION.y).rgb * lighting * heightBright * powderFactor)
                        * alpha * sunDimFactor;

            float dTrans = exp(-alpha * dD);
            float3 Sint = (S - S * dTrans) * (1.0f / max(alpha, 0.001f));

            scatteredLight += transmittance * Sint;
            transmittance *= dTrans;

            sceneDist = min(sceneDist, d);
            d += dD;
        }
        else
        {
            float distToBottom = abs(length(p - sphereCenter) - SPHERE_INNER_RADIUS);
            float distToTop = abs(length(p - sphereCenter) - SPHERE_OUTER_RADIUS);
            float sdfDist = min(distToBottom, distToTop) * 0.5f;
            sdfDist = max(sdfDist, dD);
            d += sdfDist;
        } // if - else

        if (transmittance <= CLOUDS_MIN_TRANSMITTANCE)
        {
            break;
        }
    } // for
    
    float horizonFade = smoothstep(0.0f, HORIZON_FADE_SCALE, rd.y);
    scatteredLight *= horizonFade;
    transmittance = lerp(1.0f, transmittance, horizonFade);

    return float4(scatteredLight, transmittance);
} // RaymarchClouds

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint width, height;
    OutTexture.GetDimensions(width, height);
    
    if (DTid.x >= width || DTid.y >= height)
    {
        return;
    }

    float2 uv = (float2(DTid.xy) + 0.5f) / float2(width, height);
    float3 ro = CAMERA_POSITION;
    float3 rd = ray_direction_restore(uv, PROJ_INV, VIEW_INV);
    float sceneDepth = SceneDepth.Load(int3(DTid.xy * 2, 0)).r;
    float dist = MAX_DIST;
    
    // 터레인 or 오브젝트가 있는 픽셀인지 확인
    if (sceneDepth > 0.0f)
    {
        float3 worldPos = get_world_from_depth(uv, sceneDepth, VIEW_INV, PROJ_INV);
        dist = length(worldPos - ro);
    }

    // col.rgb: 산란된 구름 빛
    // col.a: 투과율
    float4 col = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    if (rd.y > 0.0f)
    {
        col = RaymarchClouds(ro, rd, dist, DTid.xy);
        float fogAmount = 1.0f - (0.1f + exp(-dist * 0.0001));
        col.rgb = lerp(col.rgb, GetSkyColor(rd) * (1.0f - col.a), fogAmount);
    }
    
    OutTexture[DTid.xy] = col;
    OutTransmittance[DTid.xy] = col.a;
} // main