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
#define  KM  1000.0f
#define  NM_TO_M 1852
#define  FT_TO_M 0.3048
#define  LIGHT_MARCH_SIZE 400.0f
#define  MAX_LENGTH 422440.0f

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 localPos : TEXCOORD0;
}; // PS_INPUT

cbuffer AtmosphereBuffer : register(b3)
{
    // [Row 1] 단순 그라데이션
    float4 aZenithColor;
    // [Row 2] 단순 그라데이션
    float4 aHorizonColor;
    // [Row 3] 행성 물리 데이터
    float3 aPlanetCenter;
    float  aPlanetRadius;
    // [Row 4] 대기권 물리 데이터
    float  aAtmoRadius;
    float3 aPadding;
    // [Row 5] 산란 계수 (Rayleigh)
    float3 aRayleighBeta;
    float  aMieBeta;
    // [Row 6] 흡수 및 주변광
    float3 aAbsorptionBeta;
    float  aAmbientBeta;
    // [Row 7] 고도 상수 (Density Falloff)
    float  aRayleighHeight;
    float  aMieHeight;
    float  aAbsorptionHeight;
    float  aAbsorptionFalloff;
    // [Row 8] Mie 위상 함수 및 샘플링 설정
    float  aG;
    int    aPrimarySteps;
    int    aLightSteps;
    float  aIntensity;
    // [Row 9] 지표면 색상
    float3 aGroundColor;
    float  aPadding2;
    // [Row 10] 지표면 레이마칭 설정
    int    aGroundPrimarySteps;
    int    agroundLightSteps;
    float  aPadding3;
}; // AtmosphereBuffer

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
Texture2D    WeatherMapTexture : register(t2);
Texture3D    BaseNoiseTexture : register(t3);
Texture3D    DetailNoiseTexture : register(t4);

float GetDistanceCloud(float pos, float botoom, float thickness)
{
    return min(abs(pos - botoom), abs(pos - botoom) - thickness);
} // GetDistanceCloud

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

float3 monteCarloAmbient(float3 normal, float3 skyColor)
{
    const int NUM_SAMPLES_MONTE_CARLO = 16;
    float3 ambientColor = 0.0;
    for (int i = 0; i < NUM_SAMPLES_MONTE_CARLO; i++)
    {
        
        // half sphere
        float3 sampleDir = RandomDirection(normal);
        // if (dot(sampleDir, normal) < 0.0) {
        //     sampleDir = -sampleDir;
        // }
                
        ambientColor += skyColor;
    }
    return ambientColor / float(NUM_SAMPLES_MONTE_CARLO);
}
    
float CloudDensity(float3 pos, out float distance, out float3 normal, bool lowFreq = false)
{

    const float rayHeightMeter = -pos.y;
    normal = float3(0, 1, 0); // Placeholder normal
    distance = 5.0; // Placeholder distance

    float4 fmap = WeatherMapTexture.SampleLevel(LinearWrapSampler, get_box_uvw(pos, 0.0, KM * 16 * 64).xz, 0.0);
    float poor = remap_clamp(fmap.r, 0.0, 1.0, 0.0, 1.0);
    float finaldense = 1.0;

    // first later: cumulus and stratocumulus
    {
        // the narrower UV you use, the more noise but performance worse
        // the wider UV you use, the less noise but performance better
        //float4 largeNoiseValue = Noise3DTex(pos * (1.0) / (1000 * 16), 0); // Large scale noise
        float4 largeNoiseValue = BaseNoiseTexture.SampleLevel(LinearWrapSampler, pos * (1.0) / (KM * 16), 0.0f); // Large scale noise
        float dense = 1.0;
        dense = remap_clamp((largeNoiseValue.r * 0.5 + 0.5), 1.0 - poor * 0.75, 1.0, 0.0, 1.0); // perlinWorley
        if (!lowFreq)
        {
            dense = remap_clamp(dense, 1.0 - (largeNoiseValue.g * 0.5 + 0.5), 1.0, 0.0, 1.0); // worley
            dense = remap_clamp(dense, 1.0 - (largeNoiseValue.b * 0.5 + 0.5), 1.0, 0.0, 1.0); // worley
            dense = remap_clamp(dense, 1.0 - (largeNoiseValue.a * 0.5 + 0.5), 1.0, 0.0, 1.0); // worley
        }
        const float cumulusThickness = 500 + 2000 * fmap.g;
        const float cumulusBottomAltMeter = fmap.b * 0.3048;
        const float height = (rayHeightMeter - cumulusBottomAltMeter) / cumulusThickness;
        distance = GetDistanceCloud(rayHeightMeter, cumulusBottomAltMeter, cumulusThickness);
        const float cumulusLayer = remap_clamp(height, 0.00, 0.20, 0.0, 1.0) * remap_clamp(height, 0.20, 1.00, 1.0, 0.0);
        dense = remap_clamp(dense, 1.0 - cumulusLayer, 1.0, 0.0, 1.0);
        // cumulus anvil
        const float anvil = 1.0;
        const float slope = 0.2;
        const float bottomWide = 0.8;
        dense = pow(dense, remap_clamp(1.0 - height, slope, bottomWide, 1.0, lerp(1.0, 0.5, anvil)));
        finaldense = dense;
    }

    // second later: cirrus
    {        
        // the narrower UV you use, the more noise but performance worse
        // the wider UV you use, the less noise but performance better
        float4 largeNoiseValue = BaseNoiseTexture.SampleLevel(LinearWrapSampler, pos * (1.0) / (KM * 16) + 0.5, 0); // Large scale noise
        float dense = 1.0;
        dense = remap_clamp((largeNoiseValue.r * 0.5 + 0.5), 1.0 - poor * 0.5, 1.0, 0.0, 1.0); // perlinWorley
        if (!lowFreq)
        {
            dense = remap_clamp(dense, 1.0 - (largeNoiseValue.g * 0.5 + 0.5), 1.0, 0.0, 1.0); // worley
            dense = remap_clamp(dense, 1.0 - (largeNoiseValue.b * 0.5 + 0.5), 1.0, 0.0, 1.0); // worley
            dense = remap_clamp(dense, 1.0 - (largeNoiseValue.a * 0.5 + 0.5), 1.0, 0.0, 1.0); // worley
        }
        const float cumulusThickness = 10 + 500 * fmap.g;
        const float cumulusBottomAltMeter = (fmap.b + 5000) * 0.3048;
        const float height = (rayHeightMeter - cumulusBottomAltMeter) / cumulusThickness;
        distance = min(distance, GetDistanceCloud(rayHeightMeter, cumulusBottomAltMeter, cumulusThickness));
        const float cumulusLayer = remap_clamp(height, 0.00, 0.20, 0.0, 1.0) * remap_clamp(height, 0.20, 1.00, 1.0, 0.0);
        dense = remap_clamp(dense, 1.0 - cumulusLayer, 1.0, 0.0, 1.0);
        // cumulus anvil
        const float anvil = 1.0;
        const float slope = 0.2;
        const float bottomWide = 0.8;
        dense = pow(dense, remap_clamp(1.0 - height, slope, bottomWide, 1.0, lerp(1.0, 0.5, anvil)));
        finaldense = max(dense, finaldense);
    }

    // apply noise detail
    // the narrower UV you use, the more noise but performance worse
    // the wider UV you use, the less noise but performance better
    if (!lowFreq)
    {
        float4 smallNoiseValue = DetailNoiseTexture.SampleLevel(LinearWrapSampler, pos * 1.5 / (1.0 * NM_TO_M), 0); // small scale noise   
        finaldense = remap_clamp(finaldense, 1.0 - (smallNoiseValue.r * 0.5 + 0.5), 1.0, 0.0, 1.0); // worley
        finaldense = remap_clamp(finaldense, 1.0 - (smallNoiseValue.g * 0.5 + 0.5), 1.0, 0.0, 1.0); // worley
        finaldense = remap_clamp(finaldense, 1.0 - (smallNoiseValue.b * 0.5 + 0.5), 1.0, 0.0, 1.0); // worley
    }
    return finaldense / 64.0;
}

float4 RayMarch(float3 rayStart, float3 rayDir, int sunSteps, float in_start, float in_end, float2 screenPosPx, float primDepthMeter, out float output_cloud_depth, float3 skyColor)
{
    output_cloud_depth = 0;

    // 빛 방향 고정
    const float3 SUNDIR = -(cLightDirection.xyz * float3(-1, 1, -1));
    const float3 SUNCOLOR = calculate_sunlight_color(SUNDIR, cEarthRadius);

    // RGB에서의 산란, A에서의 투과
    float4 intScattTrans = float4(0, 0, 0, 1);

    // 태양광 산란
    float cos_angle = dot(normalize(SUNDIR), normalize(rayDir));
    float lightScatter = HenyeyGreenstein(dot(normalize(SUNDIR), normalize(rayDir)), 0.05);
    float rayDistance = in_start;

    bool hit = false;

    const float maxStep = 2000;
    float2 p = atmosphere_earth_intersect(rayStart, rayDir, aAtmoRadius * KM, cEarthRadius);
    const float maxLength = p.y - p.x;

    [fastopt]
    for (int i = 0; i < maxStep; i++)
    {

        // 루프마다 Ray의 위치를 ​​이동
        float3 rayPos = rayStart + rayDir * rayDistance;
        rayPos = ray_apply_curvature(rayPos, cCameraPosition.xyz, cEarthRadius);

        // 현재 위치에서의 밀도
        float distance;
        float3 normal;
        const float DENSE = CloudDensity(rayPos, distance, normal);

        float2 p = atmosphere_earth_intersect(rayPos, rayDir, aAtmoRadius * KM, cEarthRadius);
        
        // 다음 반복을 위해
        float misStep = rayDistance < 10000 ? 50 : (p.y - p.x) / (maxStep - i);
        const float RAY_ADVANCE_LENGTH = max(misStep, distance * 1.00);
        rayDistance += RAY_ADVANCE_LENGTH;

        // 뎁스 체크
        if (rayDistance > min(primDepthMeter, maxLength))
        {
            break;
        }

        // 0일경우 패스
        if (DENSE <= 0.0)
        {
            continue;
        }
        if (!hit)
        {
            // 구름의 두께를 계산
            const float4 PROJ = mul(mul(float4(rayPos - cCameraPosition.xyz, 1.0), cView), cProjection);
            output_cloud_depth = PROJ.z / PROJ.w;
            hit = true;
        }

        // 클라우드 내부에서 시작

        // 산란 및 투과를 계산
        const float TRANSMITTANCE = BeerLambert(max(DENSE, 0.0f), RAY_ADVANCE_LENGTH);
                                  //* Powder(UnsignedDensity(DENSE), RAY_ADVANCE_LENGTH);
                                  //* lightScatter;
        float lightVisibility = 1.0;

        // 빛 레이마칭
        float previousDensity = DENSE;

        [unroll]
        for (int s = 1; s <= sunSteps; s++)
        {
            const float TO_SUN_RAY_ADVANCED_LENGTH = (LIGHT_MARCH_SIZE / sunSteps);
            const float3 TO_SUN_RAY_POS = rayPos + SUNDIR * TO_SUN_RAY_ADVANCED_LENGTH * s;

            float nd;
            float3 nn;
            const float DENSE_2 = CloudDensity(TO_SUN_RAY_POS, nd, nn);
            
            // 사다리꼴 적분
            float averageDensity = (previousDensity + DENSE_2) * 0.5;
            lightVisibility *= Energy(max(averageDensity, 0.0f), TO_SUN_RAY_ADVANCED_LENGTH, lightScatter);
            previousDensity = DENSE_2;
        }

        // 산란을 통합
        float3 integScatt = lightVisibility * (1.0 - TRANSMITTANCE);
        intScattTrans.rgb += integScatt * intScattTrans.a * SUNCOLOR;
        intScattTrans.a *= TRANSMITTANCE;

        // MIP DEBUG
        // if (MipCurve(rayPos) <= 4.0) { intScattTrans.rgb = float3(1, 0, 1); }
        // if (MipCurve(rayPos) <= 3.0) { intScattTrans.rgb = float3(0, 0, 1); }
        // if (MipCurve(rayPos) <= 2.0) { intScattTrans.rgb = float3(0, 1, 0); }
        // if (MipCurve(rayPos) <= 1.0) { intScattTrans.rgb = float3(1, 0, 0); }

        if (intScattTrans.a < 0.03)
        {
            intScattTrans.a = 0.0;
            break;
        }
    }

    // ambient
    intScattTrans.rgb += monteCarloAmbient( /*ground*/float3(0, 1, 0), skyColor) * (1.0 - intScattTrans.a);
    
    // 누적된 산란 및 투과 값을 반환
    return float4(intScattTrans.rgb, 1 - intScattTrans.a);
} // RayMarch

float4 main(PS_INPUT input) : SV_TARGET
{  
    float3 rd = normalize(input.localPos); // 시선 방향
    float3 ro_atmos = cCameraPosition / KM; // 카메라 위치
    float3 ro_cloud = cCameraPosition; // 미터 단위로 변환 (계산 편의상)
    float2 uv = input.position.xy / float2(cScreenResolution.x, cScreenResolution.y);
    float depth = SceneDepthTexture.Load(int3(input.position.xy, 0)).r;

    float3 worldPos = get_world_from_depth(uv, depth, cViewInv, cProjInv);
    float max_dist = MAX_DIST;

    if (depth < 1.0f)
    {
        max_dist = length(worldPos - ro_atmos);
    }
    
    float2 planet_intersect = ray_sphere_intersect(ro_atmos - aPlanetCenter, rd, aPlanetRadius - 0.1f);
    float groundDist = (planet_intersect.x > 0) ? planet_intersect.x : MAX_DIST;
    float3 scene_color = dot(rd, cLightDirection) > 0.9998 ? 3.0 : 0.0;
    
    if (planet_intersect.y > 0.0)
    {
        max_dist = max(planet_intersect.x, 0.0);
        scene_color = calculate_ground_scattering(ro_atmos, rd, planet_intersect,
            aGroundColor, 3.0 * aAtmoRadius, ORIGIN, cLightDirection, float3(aIntensity, aIntensity, aIntensity),
            aPlanetCenter, aPlanetRadius, aAtmoRadius,
            aRayleighBeta, aMieBeta, aAbsorptionBeta, float3(aAmbientBeta, aAmbientBeta, aAmbientBeta),
            aG, aRayleighHeight, aMieHeight, aAbsorptionHeight, aAbsorptionFalloff,
            aGroundPrimarySteps, agroundLightSteps);
    }
    
    float3 col = calculate_atmosphere_scattering(
        ro_atmos, rd, max_dist,
        scene_color,
        -cLightDirection,
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
    
    float max_cloud_dist = MAX_DIST; // 400km (구름이 보이는 최대 시야거리)
    
    if (depth < 1.0f)
    {
        // 지형에 부딪혔다면, 지형까지의 거리와 400km 중 짧은 것을 선택
        max_cloud_dist = min(length(worldPos - ro_cloud), max_cloud_dist);
    }
    
    float cloudDepth = 0.0f;
    //float primDepth = SceneDepthTexture.Sample(LinearWrapSampler, pixelPos).r;
    //primDepth = min(primDepth, SceneDepthTexture.Sample(LinearWrapSampler, pixelPos + float2(+1.0, 0.0) / cPixelSize_.xy).r);
    //primDepth = min(primDepth, SceneDepthTexture.Sample(LinearWrapSampler, pixelPos + float2(-1.0, 0.0) / cPixelSize_.xy).r);
    //primDepth = min(primDepth, SceneDepthTexture.Sample(LinearWrapSampler, pixelPos + float2(0.0, +1.0) / cPixelSize_.xy).r);
    //primDepth = min(primDepth, SceneDepthTexture.Sample(LinearWrapSampler, pixelPos + float2(0.0, -1.0) / cPixelSize_.xy).r);
    //float primDepthMeter = depth_to_meter(primDepth, cProjection);
    
    float start_dist = 0.0f; // 카메라가 구름 아래에 있다면 0부터 시작
    
    // 1. 구름 레이마칭 실행
    // sunSteps: 6~8 (테스트용이므로 적당히), skyColor: 현재 대기 산란 결과물 사용
    float4 cloudResult = RayMarch(ro_cloud, rd, 8, 0, MAX_LENGTH * 0.10, uv, max_cloud_dist, cloudDepth, col);

    // 2. 최종 합성 (Alpha Blending)
    // cloudResult.rgb는 이미 누적된 산란광(Scattering)이며, cloudResult.a는 (1 - 투과도)
    // 대기색(col)에 구름의 투과도(1.0 - cloudResult.a)를 곱하고 구름 색을 더합
    float3 finalCol = col * (1.0f - cloudResult.a) + cloudResult.rgb;
    col = 1.0 - exp(-1.0 * col);
    return float4(col, 1.0f);
} // main