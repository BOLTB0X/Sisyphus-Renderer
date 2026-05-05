# Sisyphus-Renderer - Volumetric Cloud

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8403.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>AAA 급 볼류메트릭 클라우드</strong></p>
</div>

<p align="center">
  물리 기반 레이마칭과 다중 산란 근사를 통해 영화 같은 비주얼을 구현
</p>

<br/>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%84.gif?raw=true" width="320" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8404.gif?raw=true" width="320" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8402.gif?raw=true" width="320" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>라이트 레이마칭 | 다중산란 | 다이나믹 주변광</strong></p>
</div>

<table>
<tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%84.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/t_%ED%88%AC%EA%B3%BC%ED%85%8C%EC%8A%A4%ED%8A%B82.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/t_%EC%82%B0%EB%9E%80%EC%A1%B0%EC%A0%8807.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/06Volumetric_TAA6.png?raw=true" width="320"></td>
  </tr>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/04Volumetric_%EB%B3%BC%EB%A5%98%EB%A7%A4%ED%8A%B8%EB%A6%AD%EC%89%90%EB%8F%84%EC%9A%B0%EC%A1%B0%EC%A0%952.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master04_%ED%99%A9%ED%98%BC.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/06Volumetric_TAA5.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/08Volumetric_%EB%A0%8C%EC%A6%88%ED%94%8C%EB%A0%88%EC%96%B402.png?raw=true" width="320"></td>
  </tr>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8405.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8406.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8407.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8408.png?raw=true" width="320"></td>
  </tr>
</table>

<p align="center">
  하프 해상도 파이프라인과 TAA를 결합하여 나름 괜찮은 렌더러
</p>

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

- [`RenderTexture.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Data/RenderTexture.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Data/RenderTexture.cpp)

- [`TextureManager.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Components/TextureManager.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Components/TextureManager.cpp)

  - [`VolumeTexture.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Resources/VolumeTexture.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Resources/VolumeTexture.cpp)

- [`NoiseGenerator.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Data/NoiseGenerator.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Data/NoiseGenerator.cpp)

  - [`Noise.hlsli`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/HLSL/Noise.hlsli)

  - [`WorleyNoiseCS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/HLSL/WorleyNoiseCS.hlsl)

- [`CloudMap.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Data/CloudMap.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Data/CloudMap.cpp)

  - [`CloudMapCS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/HLSL/CloudMapCS.hlsl)

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

- [`VolumetricCloud.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Data/VolumetricCloud.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Data/VolumetricCloud.cpp)

  - [`Volumetric.hlsli`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/HLSL/Volumetric.hlsli)

  - [`VolumetricCloudCS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/HLSL/VolumetricCloudCS.hlsl)

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

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/t_%ED%95%A9%EC%84%B1%EC%A1%B0%EC%A0%881.png?raw=true" width="250" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/t_%ED%95%A9%EC%84%B1%EC%A1%B0%EC%A0%882.png?raw=true" width="250" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/05Volumetric_%EA%B0%80%EC%9A%B0%EC%8B%9C%EC%95%8802.png?raw=true" width="250" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/08Volumetric_%EB%A0%8C%EC%A6%88%ED%94%8C%EB%A0%88%EC%96%B402.png?raw=true" width="250" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/%EC%99%84%EC%84%B1%EB%B3%B8.png?raw=true" width="250" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong> 합성 -> 후처리 -> TAA </strong></p>
</div>

- [`PostProcess.hlsli`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/HLSL/PostProcess.hlsli)

- [`PostVS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/HLSL/PostVS.hlsl)

- [`CloudComposite.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Post/CloudComposite.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Post/CloudComposite.cpp)

  - [`CloudCompositePS.hlsli`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/HLSL/CloudCompositePS.hlsl)

- [`PostEffects.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Post/PostEffects.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Post/PostEffects.cpp)

  - [`BloomPS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/HLSL/BloomPS.hlsl)

  - [`LensflarePS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/HLSL/LensflarePS.hlsl)

  - [`GodRayPS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/HLSL/GodRayPS.hlsl)

  - [`CompositePS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/HLSL/CompositePS.hlsl)

- [`TAA.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Post/TAA.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/Post/TAA.cpp)

  - [`TemporalAntiAliasingPS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/VolumetricCloud/src/Graphics/HLSL/TemporalAntiAliasingPS.hlsl)

---

## [알면 좋은 것들]

### 1. `norY` 오프셋 튜닝


볼류메트릭 렌더링에서 구름층 내 높이 정규화(`norY`)는 보통 하단 기준으로 계산함

```cpp
// 수학적으로 올바른 방식 (하단 = 0, 상단 = 1)
float norY = (length(p - sphereCenter) - (EARTH_RADIUS + CLOUDS_BOTTOM))
             / (CLOUDS_TOP - CLOUDS_BOTTOM);
```

하지만 이 **Sisyphus Render** 에서는 상단 기준 오프셋을 사용

```cpp
// 의도적 편향 - 상단 기준 (항상 0에 가까운 값)
float norY = (length(p - sphereCenter) - (EARTH_RADIUS + CLOUDS_TOP))
             / (CLOUDS_TOP - CLOUDS_BOTTOM);
```

이렇게 하면 `norY` 가 항상 `음수 ~ 0` 범위로 떨어지고, `clamp(norY, 0, 1)` 후엔 `0`에 가까운 값이 됌

이러한 방식이 의도치 않게 세 가지 효과를 만들어냈음

- `cloud_gradient(norY ≈ 0)` : 밀도 전체가 낮아짐 -> 과포화 완화

- `compute_height_brightness(≈ 0)` : 균일한 중간 밝기 -> 구름 내부 자연스러움

- `compute_ambient_color(≈ 0)` : ambient 적분 안정 -> 경계 아티팩트 감소


<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/t_nor%EC%A0%95%ED%99%95_%EC%83%89%EC%83%81%EC%A0%95%ED%99%95.png?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/t_nor%EB%B0%98%EB%8C%80_%EC%83%89%EC%83%81%EB%B0%98%EB%8C%802.png?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
    <br>
  <p><strong> 전 / 후 </strong></p>
</div>

실시간 렌더링에서 수학적 정확성보다 시각적 결과를 우선하는 실용적 타협


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

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master.png?raw=true" width="500" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  <p><strong> 카메락 구름층 전체를 보거나 후처리에서 GodRays 등이 적용될 땐 40 까지 떨어짐 </strong></p>
</div>

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