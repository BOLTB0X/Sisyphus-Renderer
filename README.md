# Sisyphus-Renderer - Volumetric Cloud 2.0

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9505.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>AAA 급 볼류메트릭 클라우드</strong></p>
</div>

<p align="center">
  Sisyphus Engine의 볼류메트릭 클라우드 렌더링 시스템을 2.0으로 개편하여, 기존의 비물리적 편향(Hack)을 제거하고 에너지 보존 법칙(Energy Conservation)에 기반한 물리적 렌더링(PBR) 을 달성
</p>

- 전반적인 파이프라인은 [Volumetric Cloud(라이팅 과포화)](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud) 동일

- HLSL에서 `RaymarchLight` 함수를  수학적, '물리 기반 정석'으로 수정하여, `(현재 거리 - 바닥 거리) / (천장 거리 - 바닥 거리)` 공식 이용

<br/>


<table>
<tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9503.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9502.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9504.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9509.png?raw=true" width="320"></td>
  </tr>
</table>

<p align="center">
  기존 NorY 공식에서 Top 기준<br/>
  레이마칭 스텝을 줄이기 위해 비정상적으로 누적된 것이 과포화을 하기 위해서 (현재 거리 - 천장 거리) / (천장 거리 - 바닥 거리) 을 진행
</p>

---

## [기존의 '의도된 편향'](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud#1-nory-%EC%98%A4%ED%94%84%EC%85%8B-%ED%8A%9C%EB%8B%9D)

```cpp
float norY = clamp((length(p - sphereCenter) - (EARTH_RADIUS + CLOUDS_TOP)) / (CLOUDS_TOP - CLOUDS_BOTTOM), 0.0f, 1.0f);
```

- 초기 버전(1.0)에서는 구름 상단부에서 빛이 누적되어 하얗게 타버리는 **과포화(Oversaturation)** 현상을 막기 위해,

- 고도 정규화(`norY`) 계산 시 의도적으로 수식을 비틀어 사용

- 이로 인해 밀도 전체가 낮아져 과포화는 피할 수 있었으나, 구름 내부의 고도별 밀도 그라데이션이 소실되어 물리적으로 타당하지 않은 *'가짜(Fake)' 렌더링*

---

## 정석 수식 도입과 라이팅 시스템 재설계

```cpp
float norY = clamp((length(p - sphereCenter) - (EARTH_RADIUS + CLOUDS_BOTTOM)) / (CLOUDS_TOP - CLOUDS_BOTTOM), 0.0f, 1.0f);
```

2.0 업데이트에서는 고도 기준을 바닥(`Bottom`)으로 바로잡아 구름의 밀도 그라데이션을 완벽하게 복원

- **Analytical Integration** 기반 에너지 보존:

  빛의 에너지가 무한히 누적되지 않도록, 광학 두께에 따른 투과율(`dTrans`)을 계산하고 `(S - S * dTrans) / alpha` 수식을 통해 에너지가 보존되는 부드러운 산란광을 구현했

- **Beer-Lambert 법칙 기반 Raymarch Light** :

  태양광이 구름 입자를 통과할 때의 감쇠(Attenuation)를 `exp(-sampleDensity * ds * absorption)`로 계산하여, 빛의 뚫고 들어오는 깊이감(God Rays)과 부드러운 Self-Shadowing을 달성

- **Dual Henyey-Greenstein & Powder Factor** :

  태양을 바라볼 때 구름 가장자리가 은빛으로 빛나는 **Silver Lining** 효과와, 짙은 구름의 어두운 **밑면(Powder Factor)** 을 사실적으로 구현했

- **SDF 기반 빈 공간 건너뛰기** :

  레이마칭 성능 최적화를 위해, 밀도가 0인 허공에서는 구름 경계까지의 거리(SDF)를 계산해 스텝을 크게 점프(`d += sdfDist`)하도록 최적화

<br/>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9506.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>결과</strong></p>
</div>


```cpp
float RaymarchLight(float3 pos, float3 sphereCenter, float3 lightDir, float initialDensity)
{
    float ds = CLOUDS_SHADOW_MARGE_STEP_SIZE;
    float3 startPos = pos;
    float shadow = 1.0f;
    float absorption = 1.0f;

    [unroll(CLOUD_SELF_SHADOW_STEPS)]
    for (int i = 0; i < CLOUD_SELF_SHADOW_STEPS; i++)
    {
        startPos += lightDir * ds;
        float norY = clamp((length(startPos - sphereCenter) - (EARTH_RADIUS + CLOUDS_BOTTOM)) / (CLOUDS_TOP - CLOUDS_BOTTOM), 0.0f, 1.0f);
        
        if (norY > 1.0f)
            return shadow;

        // 빛 방향으로의 밀도 샘플링
        float sampleDensity = ComputeCloudDensity(startPos, norY, 0.0f);
        
        shadow *= exp(-sampleDensity * ds * absorption);
        
        ds *= CLOUDS_SHADOW_MARGE_STEP_MULTIPLY;
    }
    return shadow;
} // RaymarchLight
```

```cpp
float4 RaymarchClouds(float3 ro, float3 rd, inout float sceneDist, uint2 pixelPos)
{
    if (rd.y < 0.0f)
        return float4(0, 0, 0, 1);

    float3 sphereCenter = float3(ro.x, -EARTH_RADIUS, ro.z);

    float2 inner = compute_ray_sphere_intersect(ro, rd, sphereCenter, SPHERE_INNER_RADIUS);
    float2 outer = compute_ray_sphere_intersect(ro, rd, sphereCenter, SPHERE_OUTER_RADIUS);
    
    if (outer.y < 0.0f)
        return float4(0, 0, 0, 1);
    
    float start = max(inner.y, outer.x);
    float end = min(outer.y, sceneDist);

    if (start >= end)
        return float4(0, 0, 0, 1);
    
    float dayFactor = saturate(-LIGHT_DIRECTION.y);
    float nightFactor = saturate(LIGHT_DIRECTION.y);

    float3 currentAmbientTop = lerp(CLOUD_SUNSET_AMBIENT_COLOR_BOTTOM, CLOUDS_AMBIENT_COLOR_TOP, dayFactor);
    currentAmbientTop = lerp(currentAmbientTop, CLOUD_NIGHT_AMBIENT_COLOR_TOP, nightFactor);
    float3 currentAmbientBottom = lerp(CLOUD_SUNSET_AMBIENT_COLOR_BOTTOM, CLOUDS_AMBIENT_COLOR_BOTTOM, dayFactor);
    currentAmbientBottom = lerp(currentAmbientBottom, CLOUD_NIGHT_AMBIENT_COLOR_BOTTOM, nightFactor);
   
    float d = start;
    float dD = (end - start) / (float) CLOUD_MARCH_STEPS;
    d += dD * GetBlueNoise(pixelPos); // 지터링

    float sundotrd = dot(rd, -LIGHT_DIRECTION);
    float transmittance = 1.0;
    float3 scatteredLight = float3(0.0, 0.0, 0.0);
    sceneDist = EARTH_RADIUS;

    float phaseFunction = DualHenyeyGreenstein(sundotrd, CLOUDS_FORWARD_SCATTERING_G, CLOUDS_BACKWARD_SCATTERING_G, CLOUDS_SCATTERING_LERP) * HENYEY_GREENSTEIN_SCALE;

    for (int s = 0; s < CLOUD_MARCH_STEPS; s++)
    {
        float3 p = ro + rd * d;
        float norY = clamp((length(p - sphereCenter) - (EARTH_RADIUS + CLOUDS_TOP)) / (CLOUDS_TOP - CLOUDS_BOTTOM), 0.0f, 1.0f);
        
        float alpha = ComputeCloudDensity(p, norY, d);

        if (alpha > 0.0f)
        {
            float lightTransmittance = RaymarchLight(p, sphereCenter, - LIGHT_DIRECTION, alpha);

            float extCoeff = max(alpha * 0.1f, 0.001f);
            float3 ambientLight = compute_ambient_color(p, CLOUDS_TOP, CLOUDS_BOTTOM, extCoeff, currentAmbientTop, currentAmbientBottom);
            
            float heightBright = compute_height_brightness(norY);
            float powderFactor = lerp(1.0f, 1.0f - exp(-alpha * 2.0f), POWDER_FACTOR);
            float ms = compute_multiple_scattering(alpha, dD);
       
            float3 directionalLight = get_dynamic_light_color(LIGHT_DIRECTION.y).rgb * lightTransmittance * phaseFunction;

            float3 S = (ambientLight + directionalLight + (ms * LIGHTING_SCALE)) * alpha * powderFactor * norY;
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
            float sdfDist = max(min(distToBottom, distToTop) * 0.5f, dD);
            d += sdfDist;
        }
        
        if (transmittance <= CLOUDS_MIN_TRANSMITTANCE)
            break;
    }
    
    // 지평선 페이드 아웃
    float horizonFade = smoothstep(0.0f, HORIZON_FADE_SCALE, rd.y);
    scatteredLight *= horizonFade;
    transmittance = lerp(1.0f, transmittance, horizonFade);

    return float4(scatteredLight, transmittance);
} // RaymarchClouds
```

- [`VolumetricCloudCS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/VolumetricCloudCS.hlsl)

## [참고]

- [Guerrilla games: Nubis: Authoring Real-Time Volumetric Cloudscapes with the Decima Engine](https://www.guerrilla-games.com/read/nubis-authoring-real-time-volumetric-cloudscapes-with-the-decima-engine)

- [Patapom: Real-Time Volumetric Rendering](https://patapom.com/topics/Revision2013/Revision%202013%20-%20Real-time%20Volumetric%20Rendering%20Course%20Notes.pdf)

- [Github: RenderEngine(NadirRoGue) - OpenGL](https://github.com/NadirRoGue/RenderEngine/tree/master)

- [Github: TerrainEngine(fede-vaccaro) - OpenGL](https://github.com/fede-vaccaro/TerrainEngine-OpenGL/tree/master)

- [Github: Volumetric Cloud(chihirobelmo) - DX11](https://github.com/chihirobelmo/volumetric-cloud-for-directx11/tree/main)

- [Shadertoy: Himalayas(MdGfzh)](https://www.shadertoy.com/view/MdGfzh)

- [Shadertoy: Enscape Cube(4dSBDt)](https://www.shadertoy.com/view/4dSBDt)

- [Chris' Graphics Blog: Volumetric Rendering Part 1](https://wallisc.github.io/rendering/2020/05/02/Volumetric-Rendering-Part-1.html)