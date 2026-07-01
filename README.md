# Sisyphus-Renderer - Volumetric Cloud 2.0

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9505.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>AAA 급 볼류메트릭 클라우드</strong></p>
</div>

<p align="center">
  Sisyphus Engine의 볼류메트릭 클라우드 렌더링 시스템을 2.0으로 개편하여, 기존의 비물리적 편향(Hack)을 제거하고 에너지 보존 법칙(Energy Conservation)에 기반한 물리적 렌더링(PBR) 을 달성
</p>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/11Volumetric_2_%EC%8B%A4%EB%B2%84%EB%9D%BC%EC%9D%B4%EB%8B%9D02.png?raw=true" width="450" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/12Volumetric_2_%EC%B0%A8%ED%8F%90%ED%99%95%EC%9D%B802.png?raw=true" width="450" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>실버라이닝 및 파우더에 따른 빛 조절</strong></p>
</div>

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
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/11Volumetric_2_%EC%8B%A4%EB%B2%84%EB%9D%BC%EC%9D%B4%EB%8B%9D01.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/11Volumetric_2_%EC%8B%A4%EB%B2%84%EB%9D%BC%EC%9D%B4%EB%8B%9D03.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/12Volumetric_2_%EC%B0%A8%ED%8F%90%ED%99%95%EC%9D%B801.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9508.png?raw=true" width="320"></td>
  </tr>
</table>

<p align="center">
  기존 NorY 공식에서 Top 기준<br/>
  레이마칭 스텝을 줄이기 위해 비정상적으로 누적된 것이 과포화을 하기 위해서 (현재 거리 - 천장 거리) / (천장 거리 - 바닥 거리) 을 진행
</p>

---
## [파이프라인 구조도]

1. **전처리 - 텍스처 생성**

   - `CloudMapCS.hlsl`    -> *2D Cloud LUT (R:밀도, G:디테일, B:구름타입)*
  
   - `WorleyNoiseCS.hlsl` -> *3D Worley Noise 텍스처 (침식 도구)*

2. **메인 렌더링**
  
   - `VolumetricCloudCS.hlsl` -> *구 기반 Ray-Sphere Intersect, 낮/노을/밤 Ambient 보간, Ray Marching으로 구름 렌더링*
  
   - `Volumetric.hlsli`       -> *HG Phase, Beer-Lambert, 다중산란 등 핵심 수식*
  
   - `Noise.hlsli`            -> *FBM, Voronoi, hash 함수들*

3. **후처리**

   - `CloudCompositePS.hlsl`       -> *구름 + 씬 합성 (비네팅, 필름 노이즈)*
  
   - `TemporalAntiAliasingPS.hlsl` -> *TAA (Variance Clipping + Reprojection)*

---

### 1. Cloud LUT, 월리 노이즈 생성


```cpp
[Init 단계]
    TextureMaanger::Init()
         └─ NoiseGenerator (볼륨 노이즈 생성)

    CloudMap::Init()
        └─ RenderTexture (UAV, 1024x1024) 생성
        └─ m_mipTexture (Mipped SRV) 생성
        └─ CloudMapCS 컴파일
```

```cpp
[Generate 단계 - 1회 실행]
    CloudMap::Generate()
        └─ CloudMapCS Dispatch
             └─ Perlin-Worley 노이즈로 2D 날씨맵 생성
             └─ R: 기본 밀도, G: 디테일 가이드, B: 구름 타입
        └─ CopySubresourceRegion → m_mipTexture
        └─ GenerateMips() → mip0~6 자동 생성

    VolumetricCloud::Init()
        └─ RenderTexture (UAV, halfWidth x halfHeight) 생성
        └─ VolumetricCloudCS 컴파일
        └─ VolumetricCloudBuffer 생성
```

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/00Volumetric_%EC%9B%94%EB%A6%AC%EB%85%B8%EC%9D%B4%EC%A6%88.png?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/00Volumetric_CloudLUT_%EB%B3%B4%EB%A1%9C%EB%85%B8%EC%9D%B4.png?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
    <br>
  <p><strong> UAV -> SRV </strong></p>
</div>

- [`RenderTexture.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Data/RenderTexture.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Data/RenderTexture.cpp)

- [`TextureManager.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Components/TextureManager.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Components/TextureManager.cpp)

  - [`VolumeTexture.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Resources/VolumeTexture.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Resources/VolumeTexture.cpp)

- [`NoiseGenerator.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Data/NoiseGenerator.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Data/NoiseGenerator.cpp)

  - [`Noise.hlsli`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/Noise.hlsli)

  - [`WorleyNoiseCS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/WorleyNoiseCS.hlsl)

- [`CloudMap.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Data/CloudMap.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Data/CloudMap.cpp)

  - [`CloudMapCS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/CloudMapCS.hlsl)

---

### 2. CloudMap (UAV, SRV)

```cpp
[매 프레임 Compute 단계]
Atmosphere::Execute()
  └─ AtmosphereCS → SkyLUT (대기 산란 LUT) 생성

VolumetricCloud::Execute()
  └─ VolumetricCloudCS Dispatch (halfRes)
  │
  ├─ [입력]
  │   ├─ CloudMap SRV     (t2) - 날씨맵
  │   ├─ WorleyNoise SRV  (t3) - 3D 노이즈
  │   ├─ BlueNoise SRV    (t4) - 지터링용
  │   ├─ SkyLUT SRV       (t5) - 하늘 색상
  │   ├─ SceneDepth SRV   (t1) - 깊이 버퍼
  │   └─ VolumetricCloudBuffer (b2)
  │
  └─ [픽셀당 실행]
  │
  ├─ ray_direction_restore() : rd 복원
  │
  ├─ compute_ray_sphere_intersect()
  │   ├─ SPHERE_INNER = EARTH_RADIUS + CLOUDS_BOTTOM
  │   └─ SPHERE_OUTER = EARTH_RADIUS + CLOUDS_TOP
  │   -> start, end 결정
  │
  ├─ [레이마칭 루프 - CLOUD_MARCH_STEPS회]
  │   │
  │   ├─ ComputeCloudDensity(p, norY, d)
  │   │   ├─ GetCloudMapBase()       - 거리 기반 mip 샘플링
  │   │   ├─ cloud_gradient()        - 높이별 밀도 곡선
  │   │   └─ GetWorleyNoiseMip()     - 디테일 침식
  │   │
  │   ├─ VolumetricShadow()           [alpha > 0일 때]
  │   │   ├─ 태양 방향 4 ~6 스텝 레이마칭
  │   │   ├─ 3중 Beer's Law
  │   │   └─ depth_probability         - 구름 두께 기반 밝기
  │   │
  │   ├─ ComputeAmbientLight()
  │   │   └─ compute_ambient_color()   - 높이별 ambient 적분
  │   │
  │   ├─ compute_multiple_scattering() - 다중 산란 근사
  │   ├─ compute_height_brightness()   - 높이별 밝기
  │   ├─ powder effect                 - 가장자리 밝기
  │   │
  │   └─ 적분 (Sébastien Hillaire 방식)
  │       Sint = (S - S*dTrans) / alpha
  │       scatteredLight += transmittance * Sint
  │       transmittance *= dTrans
  │
  ├─ horizonFade - 수평선 페이드
  └─ OutTexture[xy] = float4(scatteredLight, transmittance)
```

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/02Volumetric_CS1.png?raw=true" width="280" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/02Volumetric_CS2.png?raw=true" width="280" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/02Volumetric_CS3.png?raw=true" width="280" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong> UAV -> SRV </strong></p>
</div>

- [`VolumetricCloud.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Data/VolumetricCloud.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Data/VolumetricCloud_2.0.cpp)

  - [`Volumetric.hlsli`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/Volumetric.hlsli)

  - [`VolumetricCloudCS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/VolumetricCloudCS.hlsl)

---

### 3. Post-Processing

```cpp
[매 프레임 렌더 단계 - PostProcessing]
Renderer::MainPass()
  ├─ sceneRT <- DrawGround + DrawStone + DrawSkyBox
  │
  ├─ Compute() <- VolumetricCloud::Execute()
  │
  └─ PostProcessing()
      │
      ├─ [1] CloudComposite
      │   ├─ 입력: sceneRT(SRV) + cloudRT(SRV)
      │   ├─ col = cloud.rgb + scene.rgb * cloud.a
      │   ├─ 감마 보정
      │   ├─ 비네팅
      │   ├─ 필름 노이즈
      │   └─ 출력: compositeRT
      │
      ├─ [2] Bloom, Lensflare , Gods Rays, Composite
      │   ├─ 입력: Bloom(SRV), rayRT(SRV), flareRT(SRV), CompositeRT(SRV)
      │   ├─ ACES 톤매핑
      │   ├─ 원형 블러 (20샘플)
      │   ├─ Radial Blur (스크린 공간 광선 산란)
      │   ├─ 랜즈플레어
      │   ├─ 합성
      │   └─ 출력: resRT
      │
      └─ [3] TAA
          ├─ 입력: resRT(current) + historyRT
          ├─ YCoCg 색공간 변환
          ├─ Variance Clipping (Marco Salvi)
          ├─ lerp(history, current, 0.95)
          ├─ 출력: 백버퍼
          └─ CopyResource → historyRT (다음 프레임)
```

- [`PostProcess.hlsli`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/PostProcess.hlsli)

- [`PostVS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/PostVS.hlsl)

- [`CloudComposite.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Post/CloudComposite.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Post/CloudComposite.cpp)

  - [`CloudCompositePS.hlsli`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/CloudCompositePS.hlsl)

- [`PostEffects.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Post/PostEffects.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Post/PostEffects.cpp)

  - [`BloomPS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/BloomPS.hlsl)

  - [`LensflarePS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/LensflarePS.hlsl)

  - [`GodRayPS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/GodRayPS.hlsl)

  - [`CompositePS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/CompositePS.hlsl)

- [`TAA.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Post/TAA.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/Post/TAA.cpp)

  - [`TemporalAntiAliasingPS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud_2.0/src/Graphics/HLSL/TemporalAntiAliasingPS.hlsl)

---

## [알면 좋은 것들]

### [기존의 '의도된 편향'](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud#1-nory-%EC%98%A4%ED%94%84%EC%85%8B-%ED%8A%9C%EB%8B%9D)


<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%84.png?raw=true" width="450" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/11Volumetric_2_%EC%8B%A4%EB%B2%84%EB%9D%BC%EC%9D%B4%EB%8B%9D03.png?raw=true" width="450" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong> 1.0 | 2.0 </strong></p>
</div>

```cpp
float norY = clamp((length(p - sphereCenter) - (EARTH_RADIUS + CLOUDS_TOP)) / (CLOUDS_TOP - CLOUDS_BOTTOM), 0.0f, 1.0f);
```

- 초기 버전(1.0)에서는 구름 상단부에서 빛이 누적되어 하얗게 타버리는 **과포화(Oversaturation)** 현상을 막기 위해,

- 고도 정규화(`norY`) 계산 시 의도적으로 수식을 비틀어 사용

- 이로 인해 밀도 전체가 낮아져 과포화는 피할 수 있었으나, 구름 내부의 고도별 밀도 그라데이션이 소실되어 물리적으로 타당하지 않은 *'가짜(Fake)' 렌더링*

---

### 정석 수식 도입과 라이팅 시스템 재설계

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
  <p><strong>결과(갓레이, 렌즈플레어 적용상태)</strong></p>
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


### 2. 최적화 전략

현재 내 노트북 환경이 좋지 않아 최대한 효율적으로 하려 노력함

픽셀당 수십 ~ 수백 번의 텍스처 샘플링이 필요해서 성능 최적화가 핵심

하지 않으면 프레임이 10~20 대로 떨어짐

#### (1) 하프 해상도 렌더링

```cpp
bool VolumetricCloud::Init(const InitParams& params) {
	UINT halfWidth = SharedConstants::ScreenConstants::WIDTH / 2;
  UINT halfHeight = SharedConstants::ScreenConstants::HEIGHT / 2;

  if (!m_resultRT->Init(
      params.device,
      halfWidth,
      halfHeight,     
      RenderTexture::RenderTextureType::UAV,
      DXGI_FORMAT_R16G16B16A16_FLOAT)) {
      return false;
  }
  // ...
}
```

- `VloumetricCloudCS` 를 절반 해상도로 실행하고 합성 시 업샘플링

- 연산량을 4분의 1로 줄이면서 TAA가 시간축 누적으로 품질을 보완

#### (2) SDF 기반 적응형 레이마칭

```cpp
if (alpha > 0.0f)
{
    // 구름 있음 -> 정밀 스텝
    d += dD;
}
else
{
    // 구름 없음 -> 구름층 경계까지 거리 추정 후 점프
    float distToBottom = abs(length(p - sphereCenter) - SPHERE_INNER_RADIUS);
    float distToTop    = abs(length(p - sphereCenter) - SPHERE_OUTER_RADIUS);
    float sdfDist      = min(distToBottom, distToTop) * 0.5f;
    sdfDist            = max(sdfDist, dD); // 최소 dD 보장
    d += sdfDist;
}
```

- 빈 공간에서도 고정 스텝으로 샘플링하는 건 낭비이기에 구름층 경계까지의 거리를 추정해서 빈 공간을 건너뛰어야 함

```
기존: [빈공간] -> dD -> [빈공간] -> dD -> [빈공간] -> dD -> [구름]
SDF:  [빈공간] ──────sdfDist──────> [구름 근처] -> dD -> [구름]
```

- 빈 공간 스킵으로 샘플 수를 줄이면서 구름 경계 디테일은 유지됌, 단 `sphereCenter`가 실제 Secene 스케일과 맞아야 SDF 거리가 정확

#### (3) Shadow 샘플 
   
   ```cpp
   // Volumetric.hlsli 
   #define CLOUD_SELF_SHADOW_STEPS 4
   // ...

   // VolumetricCloudCS.hlsl
   float GetWorleyNoiseMip(float3 pos, float dist) {
      // ...

      // 거리 기반 밉맵 레벨 계산
      float mipLevel = clamp(log2(dist / MIPMAP_OFFSET), 0.0f, 4.0f);
    
      // ...
      
      [unroll]
      for (int i = 0; i < 4; i++)
      {
          // mipLevel을 옥타브마다 올려서
          // 고주파 디테일을 점진적으로 제거
          // ...
      }
    
      return value / totalAmplitude;
   } // SampleOctaveNoise
   ```

   - 기존 6개 커널 × 6 스텝 구조를, 4개 커널 × 4 스텝으로 줄임

   ```cpp
   float ComputeCloudDensity(float3 pos, float norY, float dist = 0.0f, bool isShadow = false) {
      float3 ps = pos;
      float m = GetCloudMapBase(ps, norY, dist);
      m *= cloud_gradient(norY);
    
      float dstrength = smoothstep(1.0f, 0.5f, m);
    
      if (!isShadow) // Shadow 계산 시 Worley 스킵
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
   ```

   - `isShadow=true` 일 때 **Worley 노이즈 디테일 계산도 스킵**

#### (4) `Multiple Scattering` 밀도 재사용

```cpp
// 기존 - 매 옥타브마다 density 재계산 (낭비)
float density = ComputeCloudDensity(p, norY) * msDensityScale;

// 개선 - 이미 계산된 alpha 재사용
float compute_multiple_scattering(float alpha, float dd)
{
    // ...

    [unroll]
    for (int oct = 0; oct < 4; oct++) {
        ms += msAttenuation * exp(-alpha * msDensityScale * dd);
        msAttenuation  *= 0.5f;
        msDensityScale *= 0.5f;
    }

    // ...
}
```

- 기존에 `ComputeMultipleScattering` 안에서 `ComputeCloudDensity` 를 4번 추가 호출하던 걸, 이미 계산된 `alpha` 를 재사용하도록 변경

#### (5) 거리 기반 밉맵 샘플링

```cpp
float mipLevel = clamp(log2(max(dist, 1.0f) / 50000.0f), 0.0f, 6.0f);
float3 cloud   = CloudMap.SampleLevel(LinearWrapSampler, pos.xz, mipLevel).rgb;
```

- **CloudMap** 과 **Worley 노이즈** 를 멀어질수록 낮은 해상도 밉맵으로 샘플링

- 타일 경계 앨리어싱을 줄이는 동시에 원거리 샘플링 비용도 줄어듬

#### (6) TAA

```
32스텝 단독   → 지글거림 있음
128스텝 단독  → 품질 좋음, 성능 4배 무거움
32스텝 + TAA  → 움직임 중 품질 충분, 성능 유지
```

- 32스텝 + 블루노이즈 지터링 + TAA 조합으로 128스텝에 준하는 품질을 구현

- 매 프레임 지터링 위치가 달라지면서 TAA가 시간축으로 샘플을 누적

---

## [참고]

- [Guerrilla games: Nubis: Authoring Real-Time Volumetric Cloudscapes with the Decima Engine](https://www.guerrilla-games.com/read/nubis-authoring-real-time-volumetric-cloudscapes-with-the-decima-engine)

- [Patapom: Real-Time Volumetric Rendering](https://patapom.com/topics/Revision2013/Revision%202013%20-%20Real-time%20Volumetric%20Rendering%20Course%20Notes.pdf)

- [Github: RenderEngine(NadirRoGue) - OpenGL](https://github.com/NadirRoGue/RenderEngine/tree/master)

- [Github: TerrainEngine(fede-vaccaro) - OpenGL](https://github.com/fede-vaccaro/TerrainEngine-OpenGL/tree/master)

- [Github: Volumetric Cloud(chihirobelmo) - DX11](https://github.com/chihirobelmo/volumetric-cloud-for-directx11/tree/main)

- [Shadertoy: Himalayas(MdGfzh)](https://www.shadertoy.com/view/MdGfzh)

- [Shadertoy: Enscape Cube(4dSBDt)](https://www.shadertoy.com/view/4dSBDt)

- [Chris' Graphics Blog: Volumetric Rendering Part 1](https://wallisc.github.io/rendering/2020/05/02/Volumetric-Rendering-Part-1.html)

---

## [참고]

- [Guerrilla games: Nubis: Authoring Real-Time Volumetric Cloudscapes with the Decima Engine](https://www.guerrilla-games.com/read/nubis-authoring-real-time-volumetric-cloudscapes-with-the-decima-engine)

- [Patapom: Real-Time Volumetric Rendering](https://patapom.com/topics/Revision2013/Revision%202013%20-%20Real-time%20Volumetric%20Rendering%20Course%20Notes.pdf)

- [Github: RenderEngine(NadirRoGue) - OpenGL](https://github.com/NadirRoGue/RenderEngine/tree/master)

- [Github: TerrainEngine(fede-vaccaro) - OpenGL](https://github.com/fede-vaccaro/TerrainEngine-OpenGL/tree/master)

- [Github: Volumetric Cloud(chihirobelmo) - DX11](https://github.com/chihirobelmo/volumetric-cloud-for-directx11/tree/main)

- [Shadertoy: Himalayas(MdGfzh)](https://www.shadertoy.com/view/MdGfzh)

- [Shadertoy: Enscape Cube(4dSBDt)](https://www.shadertoy.com/view/4dSBDt)

- [Chris' Graphics Blog: Volumetric Rendering Part 1](https://wallisc.github.io/rendering/2020/05/02/Volumetric-Rendering-Part-1.html)