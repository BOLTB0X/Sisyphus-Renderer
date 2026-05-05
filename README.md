# Sisyphus-Renderer - Sky LUT

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/%EC%8A%A4%EC%B9%B4%EC%9D%B4%EB%B0%95%EC%8A%A401_%EC%8B%A4%EC%8B%9C%EA%B0%84%EB%B3%80%EA%B2%BD03.png?raw=true" width="550" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Precomputed Atmospheric Scattering</strong></p>
</div>

<table>
<tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT01_%EC%B9%B4%EB%A9%94%EB%9D%BC%EC%9D%B4%EB%8F%99.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT02_%EC%B9%B4%EB%A9%94%EB%9D%BC%EC%9D%B4%EB%8F%99.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT03_%EC%B9%B4%EB%A9%94%EB%9D%BC%EC%9D%B4%EB%8F%99.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT04_%EC%B9%B4%EB%A9%94%EB%9D%BC%EC%9D%B4%EB%8F%99.png?raw=true" width="320"></td>
  </tr>
</table>

<div align="center">
  <strong>매 프레임 화면 픽셀마다 Raymarching을 도는 대신</strong>, <br/> <strong>Compute Shader를 사용해 텍스처(LUT)로 구워내고(Baking) 변화가 있을 때만 갱신하는 구조</strong>
</div>

---

## [파이프라인 구조도]

```
[매 프레임 렌더 단계 - Pre-computation]
Renderer::DrawSkyBox() -> Atmosphere::Execute()
  │
  ├─ [1. 조건부 갱신 (CPU 캐싱 최적화)]
  │  ├─ 태양의 방향(LightDir)이나 카메라 높이(CamPos.y) 변화 감지
  │  ├─ 변화가 없을 경우 -> 기존 LUT 재사용 (Compute Shader 스킵)
  │  └─ 변화가 있을 경우 -> Constant Buffer 업데이트 후 Compute() 호출
  │
```

- [`Renderer.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/SkyLUT/src/Core/Renderer.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/SkyLUT/src/Core/Renderer.cpp)

```
  └─ [2. Compute Shader 디스패치 (AtmosphereCS.hlsl)]
     ├─ 스레드 그룹 할당 (화면 해상도 / 8x8)
     │
     └─ [픽셀당 실행 (DTid 기반)]
        │
        ├─ 구면 좌표계 변환 (UV -> Theta, Phi)
        │  └─ 2D UV를 3D Ray 방향(rd)으로 매핑 (Equirectangular)
        │
        ├─ 지표면 교차 판별 (Ray-Sphere Intersect)
        │  ├─ 광선이 행성(지구)에 부딪히는지 판별
        │  └─ 부딪힐 경우 `calculate_ground_scattering`으로 지면 반사광 계산
        │
        ├─ [대기 산란 레이마칭 (calculate_atmosphere_scattering)]
        │  │
        │  ├─ [Primary Ray 루프 - aPrimarySteps(32)회 반복]
        │  │  ├─ 현재 고도(Height) 계산 및 공기 밀도(Density) 추출
        │  │  ├─ Rayleigh, Mie, Absorption(오존) 광학적 깊이 누적
        │  │  │
        │  │  └─ [Light Ray 루프 - aLightSteps(8)회 반복]
        │  │     ├─ 현재 위치에서 태양을 향해 레이마칭
        │  │     ├─ 태양빛이 대기를 통과하며 소실되는 감쇄량(Attenuation) 계산
        │  │     └─ 빛의 위상 함수(Phase Function) 적용
        │  │
        │  └─ 최종 산란된 빛의 양 누적 (Total Rayleigh & Mie)
        │
        └─ [최종 텍스처 출력]
           └─ 16비트 부동소수점(R16G16B16A16_FLOAT) 렌더 타겟(UAV)에 색상 저장
```

- [`Atmosphere.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/SkyLUT/src/Graphics/Data/Atmosphere.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/SkyLUT/src/Graphics/Data/Atmosphere.cpp)

- [`Atmosphere.hlsli`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/SkyLUT/src/Graphics/HLSL/Atmosphere.hlsli)

- [`AtmosphereCS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/SkyLUT/src/Graphics/HLSL/AtmosphereCS.hlsl)

```cpp
[매 프레임 렌더 단계 - SkyBox Rendering]
Renderer::DrawSkyBox() -> SkyBox::Render()
  └─ 생성된 SkyLUT를 SRV로 바인딩하여 씬의 배경(SkyDome) 텍스처로 사용
```

- [`SkyBox.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/SkyLUT/src/Graphics/Objects/SkyBox.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/SkyLUT/src/Graphics/Objects/SkyBox.cpp)


---

## [알면 좋은 것들]

### (1) Compute Shader를 활용한 LUT 캐싱 (성능 최적화)

**무거운 다중 산란(Multiple Scattering)** 연산을 매 프레임 화면의 모든 픽셀에서 수행하는 것은 비효율적

- 이를 해결하기 위해 Compute Shader(`AtmosphereCS.hlsl`)를 이용하여 구면 좌표계($\theta$, $\phi$)를 2D UV로 펼친 뒤, **별도의 렌더 타겟에 SkyLUT로 베이킹**

- 태양의 방향(`LightDirection`)이나 카메라의 고도(`CameraPosition.y`)에 변화가 생겼을 때만 `Compute Shader`를 디스패치하여 텍스처를 갱신, 이를 통해 **동적인 시간 변화(Day/Night Cycle)** 를 가능하면서도 런타임 렌더링 부하를 극적으로 낮춤

### (2) 2 물리 기반 대기 산란

> [Atmospheric scattering explained](https://www.shadertoy.com/view/wlBXWK) 내 공식들 사용

- **Rayleigh Scattering** : 대기 중의 작은 공기 분자에 의한 산란을 계산하여, 낮 시간대의 푸른 하늘과 붉은 노을을 물리적으로 정확하게 표현

- **Mie Scattering** : 먼지나 수증기 등 비교적 큰 입자에 의한 방향성 산란을 계산하여, 태양 주변에 맺히는 강렬한 빛무리(Halo)를 구현

- **Absorption (Ozone Layer)** : 고도에 따른 오존층의 빛 흡수율(AbsorptionBeta)을 적용하여 다채롭고 사실적인 하늘 색상을 제어

---


## 참고

- [ShaderToy : Atmospheric scattering explained - wlBXWK](https://www.shadertoy.com/view/wlBXWK)

