# Sisyphus Renderer - ShadowMapping

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain02_%EC%89%90%EB%8F%84%EC%9A%B0%EB%A7%B504.gif?raw=true" width="550" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>ShadowMapping</strong></p>
</div>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/ShadowMapping/ShadowMapping01_01PCF.png?raw=true" width="270" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/ShadowMapping/ShadowMapping01_03%ED%8F%AC%EC%9D%B8%ED%8A%B8.png?raw=true" width="270" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <strong>PCF | Point</strong>
</div>

<br/>

<div align="center">

  **렌더 타겟(RTV)** 을 비활성화해서 깊이**Depth** 만 기록하도록 최적화한 부분과, 계단 현상을 줄이기 위해 **16-Tap Poisson Disk Sampling을 적용**
</div>

---

## [파이프라인 구조]

```cpp
[Pass 1: Depth Pass (그림자 맵 생성)]
Renderer::DepthPass()
  │
  ├─ [렌더 타겟 설정]
  │  ├─ RTV (Render Target View) = nullptr (픽셀 컬러 연산 생략)[cite: 12]
  │  └─ DSV (Depth Stencil View) = ShadowMapTexture 바인딩[cite: 12]
  │
  ├─ [빛의 시점(POV) 적용]
  │  └─ Directional Light의 View, Projection 매트릭스 사용[cite: 8, 12]
  │
  └─ DepthRecorder::Render()
     ├─ Vertex Shader (DEPTH_VS): 버텍스를 빛의 투영 공간으로 변환[cite: 8]
     ├─ Pixel Shader: nullptr (레스터라이저가 깊이 값만 기록)[cite: 8]
     └─ 결과: Depth 텍스처(Shadow Map) 생성 완료
```

- [`DepthRecorder.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/ShadowMapping/src/Graphics/Resources/DepthRecorder.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/ShadowMapping/src/Graphics/Resources/DepthRecorder.cpp)

```cpp
[Pass 2: Main Pass (화면 렌더링 및 그림자 맵 적용)]
Renderer::MainPass() -> Renderer::DrawGround() / DrawStone()
  │
  ├─ [그림자 리소스 바인딩]
  │  ├─ Pass 1에서 만든 Shadow Map을 셰이더 리소스(SRV)로 전달[cite: 12]
  │  └─ 하드웨어 비교 샘플러(SamplerComparisonState) 바인딩[cite: 10, 12]
  │
  └─ [Pixel Shader (GroundPS.hlsl)]
     │
     ├─ 픽셀의 월드 좌표를 빛의 투영 공간(Light Clip Space)으로 변환[cite: 10]
     │
     └─ calculate_poisson_shadow() (ShadowMap.hlsli) 호출[cite: 10]
        ├─ UV 좌표가 0~1(빛의 범위) 내부인지 검사[cite: 11]
        ├─ [16-Tap Poisson Disk 루프][cite: 11]
        │  ├─ poisson_disk[i]와 Spread 계수로 샘플링 위치 분산[cite: 11]
        │  └─ SampleCmpLevelZero()로 그림자 맵 깊이와 현재 깊이(Bias 적용) 비교[cite: 11]
        │
        └─ 조명 연산 결과(Diffuse)에 최종 가시성(ShadowFactor)을 곱하여 출력[cite: 10]
```

- [`Renderer.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/ShadowMapping/src/Core/Renderer.cpp)

- [`ShadowMap.hlsli`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/ShadowMapping/src/Graphics/HLSL/ShadowMap.hlsli)

- [`GroundPS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/ShadowMapping/src/Graphics/HLSL/GroundPS.hlsl)

---

## [알면 좋은 것들]

### (1) Depth-Only Pass (렌더링 오버헤드 최소화)

> 그림자를 생성하기 위해서는 빛의 시점에서 씬을 바라본 깊이(Depth) 정보만 필요

- 렌더 타겟(RTV)을 `nullptr`로 비활성화하고, 깊이 스텐실 뷰(`DSV`)에만 바인딩하여 불필요한 픽셀 색상 연산을 원천 차단

- `DepthRecorder` 셰이더 설정 시 픽셀 셰이더`(Pixel Shader)`를 `nullptr`로 지정하여 레스터라이저(Rasterizer) 단계에서 빠르게 깊이 값만 기록하도록 구현

### (2) Poisson Disk Sampling

> 기본적인 쉐도우 매핑에서 발생하는 외곽선 계단 현상(Aliasing)

- **16-Tap Poisson Disk**

  - 균일하게 분산된 16개의 샘플링 좌표(`poisson_disk[16]`)를 사용하여 주변 픽셀을 여러 번 샘플링함으로써,
  
  - 일반적인 **PCF(Percentage-Closer Filtering)** 보다 훨씬 부드럽고 자연스러운 그림자 외곽선(Penumbra)을 표현  

- `SamplerComparisonState`

---

## 참고

- [OpenGL Tutorial 16 - shadow mapping](https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/)

- [Rastertek DX 11 - Tutorial 41: Shadow Mapping](https://www.rastertek.com/dx11win10tut41.html)

- [티스토리 mstone8370 - [DirectX11] 쉐도우 맵과 PCF(Percentage Closer Filtering](https://mstone8370.tistory.com/25)