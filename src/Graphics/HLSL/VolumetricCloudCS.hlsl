// VolumetricCloudCS.hlsl
// // https://www.shadertoy.com/view/MdGfzh
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
#include "VolumetricCloud.hlsli"
#include "Remap.hlsli"
#include "Debugs.hlsli"

cbuffer VolumetricCloudBuffer : register(b2)
{
    // Row 1: 행성 기본 정보 (미터 단위)
    float3 vPlanetCenter;
    float  vPlanetRadius; // 6,371.0f

    // Row 2: 대기 및 구름 고도 영역
    float  vAtmoRadius;
    float  vCloudBottom;
    float  vCloudTop;
    float  vPadding1;
    
    // Row 3: 구름 밀도 및 형상 제어
    float  vCloudCoverage;
    float  vCloudDensity;
    float  vBaseEdgeSoftness;
    float  vDetailStrength;
    
    // Row 4: 빛의 산란 관련 (Henyey-Greenstein)
    float vForwardScatteringG; // 0.8f
    float vBackwardScatteringG; // -0.2f
    float vScatteringLerp; // 0.5f
    float vMinTransmittance; // 0.1f

    // Row 5: 구름 색상 (Ambient)
    float3 vAmbientTop;
    float  vPadding2;

    // Row 6: 구름 색상 (Ambient)
    float3 vAmbientBottom;
    float  vPadding3;
}; // VolumetricCloudBuffer

RWTexture2D<float4> OutTexture : register(u0);
SamplerState        LinearWrapSampler : register(s0);
Texture2D           WeatherMap : register(t1);
Texture3D           BaseNoise : register(t2);
Texture2D           BlueNoise : register(t3);
Texture2D           SkyLUT : register(t4);

#define CLOUDS_AMBIENT_COLOR_TOP    vAmbientTop
#define CLOUDS_AMBIENT_COLOR_BOTTOM vAmbientBottom
#define EARTH_RADIUS                vPlanetRadius
#define SPHERE_INNER_RADIUS         (EARTH_RADIUS + vCloudBottom)
#define SPHERE_OUTER_RADIUS         (EARTH_RADIUS + vCloudTop)
#define SPHERE_DELTA                float(SPHERE_OUTER_RADIUS - SPHERE_INNER_RADIUS)
#define SUN_DIR                     cLightDirection
#define SUN_COLOR                  (cLightAmbient.rgb * float3(1.1f, 1.1f, 0.95f))

float getDensityForCloud(float heightFraction, float cloudType)
{
    float stratusFactor = 1.0 - clamp(cloudType * 2.0, 0.0, 1.0);
    float stratoCumulusFactor = 1.0 - abs(cloudType - 0.5) * 2.0;
    float cumulusFactor = clamp(cloudType - 0.5, 0.0, 1.0) * 2.0;

    float4 baseGradient = stratusFactor * STRATUS_GRADIENT + stratoCumulusFactor * STRATOCUMULUS_GRADIENT + cumulusFactor * CUMULUS_GRADIENT;

    return smoothstep(baseGradient.x, baseGradient.y, heightFraction) - smoothstep(baseGradient.z, baseGradient.w, heightFraction);
} // getDensityForCloud

float sampleCloudDensity(float3 p, bool expensive, float lod, float3 sphereCenter)
{
    float heightFraction = get_height_fraction(p, sphereCenter, SPHERE_INNER_RADIUS, SPHERE_OUTER_RADIUS);
    if (heightFraction <= 0.0f || heightFraction >= 1.0f)
        return 0.0f;

    float3 animation = heightFraction * windDirection * 750.0f + windDirection * cTime * 1.0f;
    float2 uv = p.xz * 0.05f;
    float2 moving_uv = (p + animation).xz * 0.05f;

    float4 lowFreqNoise = BaseNoise.SampleLevel(LinearWrapSampler, float3(uv * vBaseEdgeSoftness, heightFraction), lod);
    float lowFreqFBM = dot(lowFreqNoise.gba, float3(0.625f, 0.25f, 0.125f));
    float base_cloud = remap_new(lowFreqNoise.r, -(1.0f - lowFreqFBM), 1.0f, 0.0f, 1.0f);
    
    float density = getDensityForCloud(heightFraction, 1.0f);
    //base_cloud *= density;
    base_cloud *= (density / max(heightFraction, 0.001f));

    float3 weather_data = WeatherMap.SampleLevel(LinearWrapSampler, moving_uv, lod).rgb;
    float cloud_coverage = weather_data.r * coverage_multiplier;
    //float base_cloud_with_coverage = remap_new(base_cloud, 1.0f - cloud_coverage, 1.0f, 0.0f, 1.0f);
    float base_cloud_with_coverage = remap_new(base_cloud, 1.0f, 1.0f, 0.0f, 1.0f);
    //base_cloud_with_coverage *= cloud_coverage;

    //return weather_data.r;
    return saturate(base_cloud_with_coverage);
}

float raymarchToLight(float3 startPos, float stepSize, float3 lightDir, float originalDensity, float lightDotEye, float3 sphereCenter)
{
    float ds = stepSize * 6.0f;
    float3 rayStep = lightDir * ds;
    float coneRadius = 1.0f;
    float density = 0.0f;
    float sigma_ds = -ds * absorption;
    float T = 1.0f;

    for (int i = 0; i < 6; i++)
    {
        float3 pos = startPos + coneRadius * noiseKernel[i] * float(i);
        float heightFraction = get_height_fraction(pos, sphereCenter, SPHERE_INNER_RADIUS, SPHERE_OUTER_RADIUS);
        
        if (heightFraction > 0.0f && heightFraction < 1.0f)
        {
            float cloudDensity = sampleCloudDensity(pos, density > 0.3f, float(i) / 16.0f, sphereCenter);
            if (cloudDensity > 0.0f)
            {
                T *= exp(cloudDensity * sigma_ds);
                density += cloudDensity;
            }
        }
        startPos += rayStep;
        coneRadius += (1.0f / 6.0f);
    }
    return T;
}

float4 raymarchToCloud(float3 startPos, float3 endPos, float3 bg, uint2 pixelCoord, float3 sphereCenter, out float4 cloudPos)
{
    float3 path = endPos - startPos;
    float len = length(path);
    const int nSteps = 64;
    
    float ds = len / float(nSteps);
    float3 dir = path / len;
    float3 stepDir = dir * ds;
    float4 col = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float3 pos = startPos + stepDir; // 시작점 지터링

    float lightDotEye = dot(normalize(SUN_DIR), normalize(dir));
    float T = 1.0f;
    float sigma_ds = -ds * densityFactor;
    bool entered = false;

    cloudPos = float4(0, 0, 0, 0);

    for (int i = 0; i < nSteps; ++i)
    {
        float density_sample = sampleCloudDensity(pos, true, float(i) / 16.0f, sphereCenter);
        
        if (density_sample > 0.0f)
        {
            if (!entered)
            {
                cloudPos = float4(pos, 1.0f);
                entered = true;
            }
            
            float light_density = raymarchToLight(pos, ds * 0.1f, SUN_DIR, density_sample, lightDotEye, sphereCenter);
            
            // 산란 계산
            float sc1 = henyeygreenstein(lightDotEye, -0.08f);
            float sc2 = henyeygreenstein(lightDotEye, 0.08f);
            float scattering = max(lerp(sc1, sc2, saturate(lightDotEye * 0.5f + 0.5f)), 1.0f);
            
            float powderTerm = powder(density_sample);
            float3 S = 0.6f * (lerp(lerp(CLOUDS_AMBIENT_COLOR_BOTTOM * 1.8f, bg, 0.2f), scattering * SUN_COLOR, powderTerm * light_density)) * density_sample;
            
            float dTrans = exp(density_sample * sigma_ds);
            float3 Sint = (S - S * dTrans) * (1.0f / max(density_sample, 0.0001f)); // 0 나누기 방지
            
            col.rgb += T * Sint;
            T *= dTrans;
        }

        if (T <= vMinTransmittance)
            break;
        pos += stepDir;
    }
    col.a = 1.0f - T;
    return col;
}

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
    float3 ro = cCameraPosition / KM;
    float3 rd = ray_direction_restore(uv, cProjInv, cViewInv);

    if (rd.y < 0.0f)
    {
        OutTexture[DTid.xy] = float4(0, 0, 0, 1);
        return;
    }

    float3 sphereCenter = float3(ro.x, - EARTH_RADIUS, ro.z);
    float2 iInner = get_ray_sphere_intersect(ro, rd, sphereCenter, SPHERE_INNER_RADIUS);
    float2 iOuter = get_ray_sphere_intersect(ro, rd, sphereCenter, SPHERE_OUTER_RADIUS);

    // 외곽 구름층을 아예 빗나간 경우
    if (iOuter.y < 0.0f)
    {
        OutTexture[DTid.xy] = float4(0, 0, 0, 1);
        return;
    }

    float tMin = max(0.0f, iOuter.x);
    float tMax = iOuter.y;

    if (iInner.x > 0.0f)
    {
        tMax = min(tMax, iInner.x);
    }
    else if (iInner.y > 0.0f)
    {
        tMin = max(tMin, iInner.y);
    }

    if (tMin >= tMax)
    {
        OutTexture[DTid.xy] = float4(0, 0, 0, 1);
        return;
    }

    float3 startPos = ro + rd * tMin;
    float3 endPos = ro + rd * tMax;
    float3 bg = float3(0.1f, 0.3f, 0.6f);

    float4 cloudPos;
    float4 cloudResult = raymarchToCloud(startPos, endPos, bg, DTid.xy, sphereCenter, cloudPos);

    //float3 path = endPos - startPos;
    //float len = length(path);
    //const int nSteps = 64;
    
    //float ds = len / float(nSteps);
    //float3 dir = path / len;
    //float3 stepDir = dir * ds;
    //float4 col = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    //// Bayer Filter 적용 (SV_DispatchThreadID.xy 활용)
    //int a = DTid.x % 4;
    //int b = DTid.y % 4;
    //float3 pos = startPos + stepDir; // 시작점 지터링
    
    //float d = sampleCloudDensity(startPos, true, 0, sphereCenter);
    //if (d > 0.0f)
    //{
    //    OutTexture[DTid.xy] = float4(1, 1, 1, 1); // 흰색 (구름 있음)
    //}
    //else
    //{
    //    OutTexture[DTid.xy] = float4(0, 0, 0.5f, 1); // 남색 (구름 없음)
    //}

    // 배경과 구름 합성
    OutTexture[DTid.xy] = float4(cloudResult.rgb, cloudResult.a);
    //OutTexture[DTid.xy] = float4(rd * 0.5 + 0.5, 1.0);;
} // main