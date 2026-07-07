# Sisyphus-Renderer - Water

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water05_SSR_light01.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Tamed Normal, View-Space Raymarching</strong></p>
</div>

<p align="center">
  전반적인 합성 파이프라인은 렌더 타깃 스크린 공간의 SceneMap, SceneDepthMap, SceneNormalMap 을 참조하는 포스트 프로세스 기반 컴포지트 레이아웃을 따름
</p>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/t_WaterNormal.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water07_SSR_Shadow05.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong> Water 표면 | SSR </strong></p>
</div>

- 노말 맵 변형으로 발생하는 레이 추적 실패 현상을 극복하기 위해 SSR 전용 분리형 노말 시스템(Tamed Normal)을 설계하고, 화면 외곽 영역의 열화(Streaking)를 억제하는 페일백 알고리즘을 도입

- [결과 및 시행착오](https://github.com/BOLTB0X/DirectX11-Draw/tree/main/DemoGIF/Renderer/Water)

<br/>

<table>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water05_SSR_light06.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water06_SSR_%EA%B5%B4%EC%A0%8802.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water06_SSR_%EA%B5%B4%EC%A0%8801.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water07_SSR_Shadow02.png?raw=true" width="320"></td>
  </tr>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water07_SSR_Shadow03.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water05_SSR_light05.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water01_Post-Process04.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water05_SSR_light04.png?raw=true" width="320"></td>
  </tr>
</table>

<p align="center">
  SSR, Light, Shadow
</p>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water07_SSR_Shadow01.gif?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water07_SSR_Shadow04.gif?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong> 분포 및 광원 위치에 따른 변화 </strong></p>
</div>

---
## [파이프라인 구조도]

1. **전처리 - 텍스처 생성**

   - `Water::Init` 단계에서 반사 연산에 필요한 해상도별 렌더 타깃 및 셰이더 컴파일 진행

   - 연속적인 파도 애니메이션을 위한 2개 노말 맵 스크롤링 구조 정의

2. **메인 렌더링 및 반사 추적**
  
   - [`WaterCompositePS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Water/src/Graphics/HLSL/WaterCompositePS.hlsl) -> 수면 메시 픽셀 셰이더 실행
  
      - `CalculateWaterNormal()` -> 시간축 오프셋을 활용한 월드 공간 수면 노말 벡터 계산
  
      - `RaymarchSSR2` -> 뷰 공간 선형 레이마칭 기반 충돌 검사 및 실시간 반사광 추적

3. **Composition**

   - 수면 고유 셰이딩 색상(굴절/심도 감쇄)과 SSR 반사광을 프레넬 계수($F$)에 따라 선형 보간(lerp) 합성 후 백버퍼 출력

---

### 1. Water 컴포넌트 자원 초기화


```cpp
[Init 단계]
    // 렌더러 초기화 및 수면 포스트 이펙트 셰이더 바인딩
    WaterComposite::Init()
        └─ RenderTexture (UAV/SRV, Full Resolution) 생성
        └─ WaterCompositePS 컴파일 및 상수 버퍼 생성
        └─ 파도 구현용 듀얼 노말 맵(TextureManager) 로드
```

- [`WaterComposite.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Water/src/Graphics/Post/WaterComposite.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Water/src/Graphics/Post/WaterComposite.cpp)

- [`WaterCompositePS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Water/src/Graphics/HLSL/WaterCompositePS.hlsl)


---

### 2. 매 프레임 수면 합성 및 레이마칭 파이프라인

```cpp
[매 프레임 렌더 파이프라인]
Renderer::MainPass()
  ├─ Opaque Pass (지형, 오브젝트, 스카이박스 렌더링) -> SceneMap, DepthMap, NormalMap 확보
  │
  └─ Water 패스 실행 (Water::Execute)
        │
        ├─ [입력 자원 바인딩]
        │    ├─ SceneMap SRV       (t0) - 전경 색상
        │    ├─ SceneDepthMap SRV  (t1) - 씬 깊이
        │    ├─ SceneNormalMap SRV (t2) - 씬 노말 (지형 판정용)
        │    └─ WaterNormalMap SRV (t3) - 수면 파도 노말
        │
        └─ [픽셀 셰이더 루프 - WaterCompositePS]
              │
              ├─ CalculateWaterNormal() : 타임 오프셋 기반 듀얼 노말 블렌딩
              │
              ├─ Tamed Normal 분리 (★엔지니어링 핵심)
              │    └─ ssrNormal = normalize(lerp(float3(0,1,0), worldNormal, 0.3f))
              │
              ├─ RaymarchSSR_Fixed(waterWorldPos, ssrNormal, viewDir, uv)
              │    ├─ 1. 뷰 공간(View Space) 변환 및 수면 아래 반사 레이 강제 반전 예외 처리
              │    ├─ 2. 레이마칭 루프 (RAY_MAX_STEP_SIZE)
              │    │     ├─ 투영 및 UV 좌표 변환 (Clip Space -> Screen UV)
              │    │     ├─ depth_to_meter() : 원형 깊이 버퍼 복원 및 선형 비교
              │    │     ├─ 적응형 두께 판정 (Adaptive Thickness) 및 카메라 등진 면(Back-face) 컬링
              │    │     └─ 보폭 점진적 증가 (stepSize *= 1.1f)
              │    └─ 3. 히트 실패 및 하늘 영역 판정 시 세로 줄무늬(Streaking) 방지 페일백 맵핑
              │
              └─ 최종 프레넬 합성
                   ├─ F = FresnelSchlick(dot(Normal, View), F0)
                   └─ OutColor = lerp(RefractionColor, reflectColor, F)
```
<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water02_G-Buffer.png?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water05_SSR_light05.png?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
    <br>
  <p><strong> G - Buffer </strong></p>
</div>

---

## [알면 좋은 것들]

### 노말 맵 경사각 변형으로 인한 반사 파괴 현상 해결 (Tamed Normal)

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/e_%EB%B0%98%EC%82%AC%EB%A7%B5%EC%97%90%EB%9F%AC.gif?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/t_%EB%B0%98%EC%82%AC%EB%A7%B5%EC%9D%B4%EC%95%88%EB%A7%9E%EC%9D%8C.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
</div>

#### 1. 원인)

파도의 디테일을 살리기 위해 거친 노말 맵을 그대로 SSR 반사 벡터 계산(`reflect`)에 대입하면, 반사 레이가 물밑(`reflectDir.y < 0`)으로 꺾이거나 화면 밖으로 무작위하게 튀어버리는 현상이 발생

이로 인해 충돌 판정이 통째로 실패하여 수면 반사가 완전히 사라지거나 까맣게 깨지는 에러가 발생


#### 2. 해결 방안)

프레넬 및 라이팅 연산에는 정밀한 `worldNormal`을 그대로 유지하되, SSR 레이마칭 추적용 `ssrNormal`은 수평 벡터(`float3(0, 1, 0)`) 방향으로 부드럽게 감쇠(Taming)시키는 이원화 구조를 채택

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/t_%EB%B0%98%EC%82%AC.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
</div>

---

### 화면 외곽 및 하늘 경계 영역의 세로 늘어짐(Streaking) 억제

#### 1. 원인) 

레이마칭이 화면 버퍼 바깥으로 벗어나거나 깊이가 무한대인 하늘 영역(`Depth 1.0`)을 추적할 때,

유효한 픽셀을 찾지 못해 화면 가장자리의 최종 샘플링 픽셀이 그대로 늘어나 추잡한 세로 줄무늬(Streaking Artifacts)가 형성됌

#### 2. 해결 방안)

반사 벡터의 앙각(`reflectDir.y`)에 비례하여 화면 상단의 안전 구역 UV 대역(`0.05f ~ 0.4f`)을 유동적으로 샘플링하는 하늘 페일백(Sky Fallback) 알고리즘을 구현하여 왜곡 없는 자연스러운 수평선 반사를 연출

---

### [최적화 전략]

제한된 하드웨어 환경 내에서 화면 공간 내의 수많은 픽셀이 깊이 버퍼를 반복 조회해야 하므로, 연산 오버헤드를 줄이기 위한 단계별 최적화 진행

#### (1) 보폭 점진적 확장 (Progressive Step Scaling)

```cpp
// 루프 내부에서 탐색 보폭을 점진적으로 가속
stepSize *= 1.1f;
```
- 레이가 카메라와 가까운 근거리 구역에서는 촘촘한 간격(`stepSize = 0.5f`)으로 정밀하게 충돌을 탐색하고, 거리가 멀어질수록 탐색 보폭을 10%씩 증가

#### (2) 거리 비례 적응형 두께 판정 (Adaptive Thickness)

```cpp
float adaptiveThickness = 0.2f + rayDistance * 0.05f;
```

- **고정된 두께(Thickness)** 값을 사용하면 원거리 구역에서 투형 깊이 버퍼의 정밀도 저하로 인해 레이가 지형을 그대로 뚫고 지나가는(Over-shooting) 현상이 발생

- 추적 거리에 비례하여 충돌 인정 두께를 선형적으로 확장함으로써 원거리 지형 충돌 누락을 방지하고 불필요한 루프 낭비를 차단

#### (3) 스크린 공간 백페이스 컬링 (Screen-Space Backface Culling)

```cpp
if (dot(reflectDir, hitNormal) <= 0.0f) { ... isHit = true; }
```

레이가 복잡한 지형 오브젝트의 내부나 카메라를 등지고 있는 뒷면 지형에 충돌했을 때,

반사광으로 표현되어서는 안 되는 데이터이므로 픽셀 노말 데이터(`SceneNormalMap`)의 내적 확인 알고리즘을 추가하여 부정확한 충돌 판정을 수정

---

## [참고]

- [Kona's Engine Dev: Screen Space Reflections in View Space](https://wallisc.github.io/)

- [EA Frostbite: Stochastic Screen-Space Reflections](https://www.ea.com/news/stochastic-screen-space-reflections)

- [Willpgfx - Screen Space Glossy Reflections](https://willpgfx.com/2015/07/screen-space-glossy-reflections/)

- [Github: TerrainEngine(fede-vaccaro) - OpenGL](https://github.com/fede-vaccaro/TerrainEngine-OpenGL/tree/master)

- [Rastertek: Tutorial 16: Small Body Water](https://www.rastertek.com/tertut16.html)

- [Rastertek: Tutorial 30: Reflection](https://www.rastertek.com/dx11win10tut30.html)

- [Rastertek: Tutorial 50: Deferred Shading](https://www.rastertek.com/dx11win10tut50.html)