# Sisyphus-Renderer - Volumetric Fog

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B81.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Sphere Bounding 기반 Volumetric Fog</strong></p>
</div>

<p align="center">
  지형에 종속되지 않고 씬 내 원하는 공간에 자유롭게 배치할 수 있는 구형 바운딩(Sphere Bounding) 구조를 채택하였으며, 3D Worley Noise와 Dual-Lobe 위상 함수(Phase Function)를 결합
</p>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B89.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B88.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <p><strong>Lighting 에 따른 변화</strong></p>
</div>


<table>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/Fog01_01%EB%B0%A4.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/Fog04_%EA%B5%AC%EA%B5%90%EC%B0%A801.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/Fog04_%EA%B5%AC%EA%B5%90%EC%B0%A803.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/Fog04_%EA%B5%AC%EA%B5%90%EC%B0%A806.png?raw=true" width="320"></td>
  </tr>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/t_%EB%B0%80%EB%8F%84%ED%99%95%EC%9D%B8.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/Fog04_%EA%B5%AC%EA%B5%90%EC%B0%A807.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/Fog04_%EA%B5%AC%EA%B5%90%EC%B0%A804.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/Fog04_%EA%B5%AC%EA%B5%90%EC%B0%A805.png?raw=true" width="320"></td>
  </tr>
</table>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B83.gif?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B87.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>지형에 종속 X</strong></p>
</div>

- [결과 및 시행착오](https://github.com/BOLTB0X/DirectX11-Draw/tree/main/DemoGIF/Renderer/Fog)


<br/>

---

## [파이프라인 구조도]

1. **전처리 - 버퍼 및 노이즈 텍스처 로드**

   - `FogComposite::Init` 단계에서 포스트 프로세싱 쿼드(Quad)를 위한 렌더 타겟(`m_compositeRT`) 생성

   - *3D Worley Noise 볼륨 텍스처* 와 *2D Noise 텍스처* 를 SRV로 바인딩하여 밀도 연산의 기저 데이터 확보
   
   <div align="center">
    <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/00Volumetric_%EC%9B%94%EB%A6%AC%EB%85%B8%EC%9D%B4%EC%A6%88.png?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
    <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/SisyphusEngine/assets/Noise/noise.png?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
    <br/>
    <p><strong> 3D Worley Noise | 2D </strong></p>
   </div>

2. **메인 렌더링 - 물리 기반 레이마칭 (Raymarching)**

   - `FogCompositePS.hlsl` -> 픽셀 셰이더 기반 화면 공간 레이마칭 실행

   - 카메라 뷰 광선을 복원(`ray_direction_restore`)하고, 씬 깊이(`Depth`) 버퍼와 충돌 시 마칭 거리를 제한하여 불필요한 연산 방지

     - **1차 마칭 (밀도 추적)**
     
        구형 바운딩 박스(`FOG_SPHERE_RADIUS`) 내부를 탐색하며 밀도(`Density`) 누적

     - **2차 마칭 (빛 추적)**
     
        광원 방향으로 추가 레이를 발사(`RaymarchLight`)하여 안개 스스로 만들어내는 그림자(`Self-Shadowing`) 연산

   <div align="center">
    <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/t_fog.png?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
    <br/>
    <p><strong> Composition 전 </strong></p>
   </div>

3. **Composition**

   - 누적된 투과율(Transmittance)과 산란광(In-scattering)을 원본 씬(`SceneTex`)에 **Alpha Blending하여 백버퍼 출력**


   <div align="center">
    <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B84.gif?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
    <br/>
    <p><strong> Composition 후 </strong></p>
   </div>

<br/>

### 1. Fog 컴포넌트 자원 초기화 및 상수 버퍼 제어

```cpp
[Init 및 GUI 제어 단계]
    // 볼류메트릭 포그 파이프라인 준비
    FogComposite::Init()
        ├─ 3D Worley Noise, 2D Base Noise 로드 및 바인딩
        └─ 독립적인 2개의 Constant Buffer (VolumetricFogBuffer, DeferredFogBuffer) 생성

    // ImGui 실시간 제어 최적화
    FogComposite::UpdateVolumetricFogBuffer()
        └─ memcmp를 활용하여 데이터 변경이 발생한 버퍼만 선별적으로 UpdateSubresource 처리 (GPU 버스 병목 차단)
```

- [`FogComposite.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Volumetric_Fog_Light/src/Graphics/Post/FogComposite.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Volumetric_Fog_Light/src/Graphics/Post/FogComposite.cpp)

- [`VolumetricFogPS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Volumetric_Fog_Light/src/Graphics/HLSL/VolumetricFogPS.hlsl)

---

### 2. 매 프레임 볼류메트릭 렌더링 파이프라인

```cpp
[매 프레임 렌더 파이프라인]
Renderer::FogPass()
  ├─ [입력 자원 바인딩]
  │    ├─ WaterComposite의 산출물 (t0) - 전경 색상
  │    ├─ SceneDepthMap SRV (t1) - 씬 깊이 (거리 제한용)
  │    ├─ Noise 2D/3D (t2, t4), TerrainShadowMap (t11)
  │
  └─ [픽셀 셰이더 루프 - FogCompositePS]
       │
       ├─ Dual-Lobe 위상 함수 계산 (빛의 맺힘과 퍼짐)
       │    ├─ phaseCore = henyey_greenstein(sundotrd, FOG_PHASE_G)
       │    └─ phaseBroad = henyey_greenstein(sundotrd, FOG_PHASE_G * 0.3f)
       │
       ├─ [RaymarchFog] 레이마칭 루프 (FOG_MARCH_STEPS)
       │    ├─ 1. ComputeFogDensitySphere() : 구형 바운딩 박스 내부의 노이즈 밀도 계산
       │    ├─ 2. RaymarchLight() : 태양광을 향해 2차 레이를 발사하여 매질의 빛 흡수율(Beer's Law) 계산
       │    └─ 3. GetFogShadow() : 지형 섀도우 맵을 샘플링하여 지형 그림자 영역의 포그를 어둡게 처리
       │
       └─ 최종 합성
            └─ color = SceneColor * Transmittance + FogScattering
```

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B82.gif?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  <p><strong> </strong></p>
</div>

## [알면 좋은 것들]

### 1. 빛 과합성(Blow-out) 제어 및 위상 함수(Phase Function) 클램핑

#### 원인)

Henyey-Greenstein 위상 함수는 태양(광원)을 정면으로 바라볼 때 극단적으로 높은 수치를 반환

이를 제한 없이 레이마칭 누적 연산에 곱하게 되면, 포그 덩어리가 거대한 발광체처럼 변하여 **화면 전체가 하얗게 타버리는 현상이 발생**

#### 해결 방안)

볼류메트릭 포그는 '빛의 매질'로서 은은한 실루엣만 제공하고, 강렬한 눈부심은 후속 **Post-Process(GodRay, Bloom)` 패스에 위임하도록 설계**

```cpp
// 빛의 퍼짐(Phase)이 일정 수치를 넘어가지 않도록 방어
float phase = lerp(phaseBroad, phaseCore, 0.5f);
phase = min(phase, 2.0f); 

// 산란 강도를 낮춰 갓레이(GodRay)와 역할을 확실히 분리
float scatteringIntensity = 0.2f; 
float3 absorption = lightContrib * density * FOG_COLOR * scatteringIntensity;
```

---

### 2. 지형 종속성 탈피 및 Local Sphere Bounding 도입

#### 원인)

초기 포그 시스템은 G-Buffer의 `Terrain Normal`의 Y축(`Slope`)과 높이(`Height`)에 비례하여 밀도를 생성

그러나 이 방식은 안개가 지형의 바닥이나 평지에만 카펫처럼 찰싹 달라붙어, 하늘에 떠 있는 뭉게구름 같은 입체적인 볼륨감을 렌더링할 수 없는 구조적 한계를 느낌

#### 해결 방안)

지형 노말 의존성을 과감히 제거하고, 월드 공간 상의 임의의 구형 영역(Sphere Center, Radius) 내부에만 밀도를 가두는 코드로 전면 교체

```cpp
// 구의 중심으로부터 멀어질수록 지수 함수로 자연스럽게 소멸
float distFromCenter = length(p - FOG_SPHERE_CENTER);
float sphereFalloff = exp(-pow(distFromCenter / FOG_SPHERE_RADIUS, 2.0f));

return FOG_DENSITY * sphereFalloff * heightFalloff * noiseFactor;
```

## [최적화 전략]

레이마칭은 픽셀 셰이더 내에서 막대한 루프 연산을 동반하므로, 프레임 레이트 방어를 위한 극한의 최적화가 필요함

### 셀프 섀도우(Self-Shadow)를 위한 Light Marching 스텝 제한

볼륨 내부의 그림자를 얻기 위해 모든 1차 마칭 스텝(`예: 32번`)마다 태양 방향으로 깊은 2차 마칭을 수행하면 연산량이 기하급수적으로 폭증하게 됌

2차 빛 추적(`RaymarchLight`)의 최대 거리를 짧게 제한(`FOG_LIGHT_MARCH_DIST`)하고 스텝 수를 최소화(4~6회)하되, 빛의 흡수율 계수를 4배(* 4.0f)로 증폭시킴

---

## 참고

- [Shadertoy - volumetric fog - above and below (trapzz)](https://www.shadertoy.com/view/lsc3Ds)

- [Shadertoy - Volumetric fog ride(sergei_am)](https://www.shadertoy.com/view/7s3SRH)

- [Github - VLD3D11 (this project is still in progress)](https://github.com/ACskyline/VLD3D11)
