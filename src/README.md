# Summery

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC11.gif?raw=true" width="230" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC12.gif?raw=true" width="230" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC13.gif?raw=true" width="230" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Sisyphus Renderer</strong></p>
</div>


```cpp
// Pseudocode of Main Rendering Loop
Renderer::Render()
{
    1. UpdatePlacement();     // GPU Grass 및 지형 컴퓨트 셰이더 연산
    2. ShadowPass();          // Object (Opaque), Terrain, Instancing Shadow Map 생성
    3. MainPass();            
       - DrawTerrain()        // QuadTree Culling & Tessellation
       - Opaque Queue         // Assimp Model 등 불투명 객체 렌더링
       - DrawSkyBox()         // 카메라 위치 기반 Atmospheric Scattering
       - DrawGPUGrass()       // 간접 그리기(Indirect Draw) 및 인스턴싱
       - Transparent Queue    // 반투명 객체 렌더링 
       - ComputeShaderData()  // Atmosphere LUT 및 Volumetric Cloud 병렬 연산
    4. CloudPass();           // Cloud 레이마칭 결과와 Scene 합성
    5. WaterPass();           // SSR(Screen Space Reflection) 및 굴절 처리
    6. FogPass();             // 지수 감쇠(Decay) 기반 Volumetric Fog 합성
    7. PostProcessingPass();  // Lens Flare, Bloom, TAA(Temporal AA) 후처리 및 최종 화면 출력
}
```

## Directory Structure

```cpp
src/
 ├── Core/           // 엔진의 핵심 기반 시스템 (타이머, 프레임 레이트, 렌더러 상태 및 큐 관리)
 ├── Graphics/       // DirectX 11 그래픽스 파이프라인 및 렌더링 리소스
 │   ├── Components/ // 카메라, 방향광, 텍스처 등 컴포넌트 단위 관리
 │   ├── D3D11/      // DX11 디바이스, 컨텍스트, 리소스 코어 래핑
 │   ├── Data/       // 컴퓨트 셰이더 연산용 데이터 버퍼
 │   ├── HLSL/       // 셰이더 소스 코드 (VS, PS, CS, HS, DS, GS)
 │   ├── Objects/    // 렌더링 대상 액터 (Terrain, MayaActor, SkinnedActor, GPUGrass 등)
 │   ├── Post/       // 화면 공간 후처리 (Fog, Cloud, Water Composite, Effects)
 │   └── Resources/  // 텍스처, 메쉬(Assimp) 등 리소스 데이터
 ├── System/         // 윈도우 생성, 시스템 루프, 입력(Input) 처리
 └── Utils/          // ImGui, 공통 수학/유틸리티 상수(Constants) 및 헬퍼 클래스
 ```

## Volumetric: [Volumetric Cloud 1.0](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud)
<details>
<summary> Summary </summary>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC01.gif?raw=true" width="460" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Volumetric Cloud 1.0</strong></p>
</div>


> 언리얼 엔진 실행 불가, 유니티 버벅이는 저사양 노트북 환경 기준

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/t_%EA%B0%93%EB%A0%88%EC%9D%B43.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>

  | 상황 | FPS |
  |---|---|
  | 일반 구름 뷰 | 50 ~ 59 |
  | 구름층 전체 + GodRays 등 후처리 풀가동 | 39 ~ 49 |

</div>


Web API(Shadertoy) 기반의 유사 구현체들이 브라우저 환경에서 20~30 FPS를 기록하는 것과 대비하면 상당한 차이가 있다 생각함

</details>

---

### Volumetric: [Volumetric Cloud 2.0](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud_2.0)

<details>
<summary> Summary </summary>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9506.gif?raw=true" width="460" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Volumetric Cloud 2.0</strong></p>
</div>

> 정석 Volumetric

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9504.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9509.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>

  레이마칭 스텝을 줄이기 위해 비정상적으로 누적된 것이 과포화을 하기 위해서 `(현재 거리 - 천장 거리) / (천장 거리 - 바닥 거리)` 을 진행했으나, <br/> 빛 레이마칭 수정으로 수학적으로 올바른 `(현재 거리 - 바닥 거리) / (천장 거리 - 바닥 거리)` 적용
</div>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8409.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9508.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  수정 전 | 수정 후
</div>

바닥(`Bottom`)에서 0, 천장(`Top`)에서 1로 정상적인 고도 그라데이션이 형성되도록 수정

</details>

---

## Volumetric: [Volumetric Fog](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Volumetric_Fog_Light)

<details>
<summary> Summary </summary>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC15.gif?raw=true" width="460" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Volumetric Fog</strong></p>
</div>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B8_%EB%9D%BC%EC%9D%B4%ED%8C%85.gif?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  포그와 갓레이
</div>

**갓레이 보완** 및 Sphere Bounding 구조로 *3D Worley Noise* 와 *Phase Function* 를 결합한 볼류매트릭 포그

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/t_%ED%92%80.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B8_%EC%A0%80%EB%85%812.gif?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  Terrain Normal | Sphere Bounding
</div>

```cpp
float distFromCenter = length(p - FOG_SPHERE_CENTER);
float sphereFalloff = exp(-pow(distFromCenter / FOG_SPHERE_RADIUS, 2.0f));

return FOG_DENSITY * sphereFalloff * heightFalloff * noiseFactor;
```

| 구분 | 렌더링 알고리즘 | 시각적 결과 및 장점 |
| :--- | :--- | :--- |
| **Terrain Normal** | 지형 노말(Y축 경사도)에 비례한 밀도 감쇠 | 지형 형태에 종속되어 입체적인 구름 표현 불가 |
| **Sphere Bounding** | 중심점(`Center`)과 반지름(`Radius`) 기반 지수 감쇠 | 하늘에 떠 있는 뭉게구름이나 마법진 등 특정 구역을 위한 입체적 볼륨 생성 가능 |

</details>

---

## CPU Based: [Quadtree Terrain & LOD Grass](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Grass)

<details>
<summary> Summary </summary>


<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC02.gif?raw=true" width="460" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Quadtree Terrain & LOD Grass</strong></p>
</div>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass03_%EB%B2%94%EC%9C%84%EC%A4%84%EC%9E%8404.gif?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass02_%EC%A1%B0%EB%AA%85.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>

  <p><strong> </strong>HeightMap 적용 Terrain + Grass Level of Detail(Geometry + Instancing, Billboard) </strong></p>
</div>

- Heightmap 기반 지형 생성 (QuadTree + Frustum Culling)

- 이중 Shadow Map 구조: `ObjectShadowMap`, `TerrainShadowMap`

- Poisson Disk Sampling PCF 소프트 섀도우

| 거리 구분 | 렌더링 방식 | 특징 |
| :--- | :--- | :--- |
| 0 ~ D | Geometry Shader | 3방향 빌보드 적용, 바람 애니메이션 효과 |
| D ~ D*2 | Hardware Instancing | Y축 고정 빌보드 적용, 밉맵 LOD 사용 |
| D*2 ~ | Ground 텍스처 블렌딩 | 풀(Grass) 렌더링 없음 (텍스처로 대체) |


</details>

---

## GPU Based: [Tessellation](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Tessellation)

<details>
<summary> Summery </summary>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain05_02.gif?raw=true" width="460" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Tessellation Terrain</strong></p>
</div>

**CPU 병목 제거** 및 동적 기하 분할을 위해 *Hull / Domain Shader* 와 *HeightMap Displacement* 를 결합한 하드웨어 테셀레이션 지형

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/t_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%98_%EC%99%80%EC%9D%B4%ED%84%B001.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/t_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%98_%EC%99%80%EC%9D%B4%ED%84%B002.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  동적 폴리곤 분할 (Wireframe)
</div>

```cpp
// 카메라 거리에 따른 비선형 테셀레이션 분할 계수 산출 (TerrainHS.hlsl)
float dist = distance(patchCenterPos, CAMERA_POSITION);
float tess = saturate((MAX_TESS_DISTANCE - dist) / (MAX_TESS_DISTANCE - MIN_TESS_DISTANCE));

return lerp(MIN_TESS_FACTOR, MAX_TESS_FACTOR, tess);
```

| 구분 | 렌더링 알고리즘 | 시각적 결과 및 장점 |
| :--- | :--- | :--- |
| **Static Mesh** | 고정된 고해상도(High-poly) 정점 데이터를 CPU가 매 프레임 파이프라인에 전송 | 막대한 대역폭(Bandwidth) 낭비 및 메모리 과부하, 거리별 디테일 조절 불가 |
| **HW Tessellation** | 단순한 제어점(Quad Patch)만 전송 후 GPU 내부 고정 하드웨어에서 거리 비례 동적 분할 | CPU-GPU 간 버스 병목 완전 해소, 근경 디테일 극대화 및 하드웨어 레벨의 완벽한 LOD 달성 |


</details>

---

##  GPU Based: [GPU Driven Grass (Instanced Foliage)](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/GPU_Driven_Grass)

<details>

<summary> Summery </summary>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC14.gif?raw=true" width="460" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>GPU Drive Grass / Instanced Foliage </strong></p>
</div>


**CPU 부하를 줄이기 위해 컴포지트 연산** 및 *StructuredBuffer*를 활용한 *GPU-Driven Rendering* 파이프라인

```cpp
// PlacementSeedCS.hlsl
// ...
// 컴퓨트 셰이더 기반의 동적 배포
float randomVal = hash_uint2(DTid.xz + i * 123);
if (randomVal > currentDensity) continue; // 밀도에 따른 확률적 배치

// DrawInstancedIndirect를 활용한 GPU 부하 분산
context->DrawInstancedIndirect(m_grassArgsBuffer.Get(), 0);
```

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Grass/GPU_Driven_Grass05.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Grass/GPU_Driven_Grass03.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  Grass | Wood
</div>

| 구분 | 렌더링 알고리즘 | 시각적 결과 및 장점 |
| :--- | :--- | :--- |
| **CPU Instancing** | CPU가 배치 연산 후 `DrawIndexedInstanced`를 호출하여 매 프레임 버퍼 전송 | 배치 개수 증가 시 CPU 메모리 복사 및 드로우 콜 병목 심화 |
| **GPU Driven** | 컴퓨트 셰이더가 직접 `StructuredBuffer`를 채우고 GPU가 Indirect로 렌더링 | CPU 개입 없이 수백만 개의 객체 처리, 거리 기반 실시간 컬링으로 대역폭 최적화 |


</details>

---

## [GodRay](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/GodRays)

<details>
<summary> Summary </summary>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/%EA%B0%93%EB%A0%88%EC%9D%B4.gif?raw=true" width="460" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>God Ray</strong></p>
</div>

**퍼포먼스 튜닝**을 위해 3D 볼륨 연산을 배제하고, 광원 스크린 좌표 기준의 *Radial Blur*와 *Decay* 알고리즘을 결합한 최적화된 빛 산란 파이프라인

```cpp
// GodRayPS.hlsl
// 광원 방향을 향해 화면 공간 텍스처 좌표를 이동하며 샘플링 누적
float2 deltaTexCoord = (input.uv - LIGHT_UV) * (1.0f / NUM_SAMPLES * DENSITY);

[loop]
for (int i = 0; i < NUM_SAMPLES; i++)
{
    uv -= deltaTexCoord;
    
    // 깊이 버퍼를 통해 하늘 영역 판별 및 구름 투과도(Transmittance)
    float cloudT = TransmittanceTex.SampleLevel(LinearSampler, uv, 0).r;
    float isSky = (DepthTex.SampleLevel(LinearSampler, uv, 0).r <= 0.0001f) ? 1.0f : 0.0f;
    cloudT *= isSky;
    
    // 빛의 누적 및 거리 기반 수학적 감쇠(Decay)
    color += InputTex.SampleLevel(LinearSampler, uv, 0).rgb * cloudT * illuminationDecay * WEIGHT;
    illuminationDecay *= DECAY;
}
```

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp01_Tree.png?raw=true" width="230" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B8_%EB%9D%BC%EC%9D%B4%ED%8C%85.gif?raw=true" width="230" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master06_%EA%B0%93%EB%A0%88%EC%9D%B4.png?raw=true" width="230" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  <p><strong>Occlusion | Scattering | Blow-out</strong></p>
</div>


</details>

---

## [Water : SSR](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Water)

<details>
<summary> Summary </summary>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/t_%EB%AC%BC%20%EB%86%92%EC%9D%B4%EC%A1%B0%EC%A0%88.gif?raw=true" width="460" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Water</strong></p>
</div>


<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water02_G-Buffer.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  G 버퍼
</div>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Water/t_%EB%B0%98%EC%82%AC.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  SSR
</div>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Water/Water07_SSR_Shadow04.gif?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
   <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water06_SSR_%EA%B5%B4%EC%A0%8803.png?raw=true" width="340" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  SSR와 Refraction 그리고 Lighting이 결합된 물 표면 렌더링
</div>


**레이마칭(Ray-Marching)을 이용한 픽셀 단위 반사** 및 *Fresnel* 항을 적용한 실시간 수면 렌더링 파이프라인


```cpp
// WaterPS.hlsl
// SSR 반사 벡터 산출
float3 reflectionVector = reflect(-viewDir, normal);
float3 rayPos = viewPos;
float3 rayStep = reflectionVector * STEP_SIZE;

// 레이마칭을 통해 화면 공간 내 충돌 지점 검색
for(int i = 0; i < MAX_STEPS; ++i) {
    rayPos += rayStep;
    float4 screenPos = mul(float4(rayPos, 1.0f), cProj);
    // ... 깊이 테스트 후 반사 색상 샘플링
}
```

</details>

---

## [Atmospheric scattering](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/SkyLUT)

<details>
<summary> Summary </summary>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/%EB%8C%80%EA%B8%B0%EC%82%B0%EB%9E%80.gif?raw=true" width="460" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Atmospheric scattering</strong></p>
</div>

**물리 기반 대기 렌더링**을 위한 *Rayleigh* 및 *Mie* 산란 [ShaderToy: Atmospheric scattering explained - skythedragon](https://www.shadertoy.com/view/wlBXWK) 을 참고

- **큐브맵 방식**

   ```cpp
   void Atmosphere::Bake(ID3D11DeviceContext* context, D3D11State* states, const RenderParams& params) {
    m_targetIdx = (m_activeIdx + 1) % 2;
    m_blendFactor = 0.0f;
    m_isInterpolating = true;

    ID3D11RenderTargetView* pPrevRTV = nullptr;
    ID3D11DepthStencilView* pPrevDSV = nullptr;
    context->OMGetRenderTargets(1, &pPrevRTV, &pPrevDSV);

    // 큐브맵 베이킹을 위한 상태 설정
    context->OMSetDepthStencilState(states->GetDepthNone(), 0);
    context->RSSetState(states->GetCullNone());

    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // 버퍼 업데이트
    UpdateLightBuffer(context, params.diffuse, params.lightDir);
    UpdateAtmosphereBuffer(context);

    // 큐브맵 전용 뷰포트 설정
    D3D11_VIEWPORT cubeVP = { 0.0f, 0.0f,
        (float)ScreenConstants::CUBE_MAP_SIZE, (float)ScreenConstants::CUBE_MAP_SIZE, 
        0.0f, 1.0f };
    context->RSSetViewports(1, &cubeVP);

    for (unsigned int i = 0; i < 6; ++i) {
        PrepareFaceRender(context, i, params); // 면 설정 분리

        m_cubeMaps[m_targetIdx]->RenderBuffer(context);
        context->DrawIndexed(m_cubeMaps[m_targetIdx]->GetIndexCount(), 0, 0);
    }

    context->OMSetRenderTargets(1, &pPrevRTV, pPrevDSV);

    if (pPrevRTV) pPrevRTV->Release();
    if (pPrevDSV) pPrevDSV->Release();
   } // Bake
   ```

- Sky LUT 방식

   ```cpp
   // AtmosphereCS.hlsl
   [numthreads(8, 8, 1)]
   void main( uint3 DTid : SV_DispatchThreadID )
   {
      uint width, height;
      OutTexture.GetDimensions(width, height);

      if (DTid.x >= width || DTid.y >= height )
      {
          return;
      }

      float2 uv = (float2(DTid.xy) + 0.5f) / rResolution;
      float theta = (uv.x - 0.5f) * 2.0f * PI; // -PI ~ PI
      float phi = (0.5f - uv.y) * PI; // -PI/2 ~ PI/2
    
      float3 rd = float3(cos(phi) * sin(theta), sin(phi), cos(phi) * cos(theta));
      float3 ro = cCameraPosition / KM;
      float max_dist = MAX_DIST;

      float2 planet_intersect = ray_sphere_intersect(ro - aPlanetCenter, rd, aPlanetRadius - 0.1f);
      float groundDist = (planet_intersect.x > 0) ? planet_intersect.x : max_dist;
      float3 scene_color = dot(rd, cLightDirection) > 0.9998 ? 3.0 : 0.0;

      if (planet_intersect.y > 0.0f)
      {
          max_dist = max(planet_intersect.x, 0.0);
          scene_color = calculate_ground_scattering(ro, rd, planet_intersect,
            aGroundColor, 3.0 * aAtmoRadius, ORIGIN, cLightDirection, float3(aIntensity, aIntensity, aIntensity),
            aPlanetCenter, aPlanetRadius, aAtmoRadius,
            aRayleighBeta, aMieBeta, aAbsorptionBeta, float3(aAmbientBeta, aAmbientBeta, aAmbientBeta),
            aG, aRayleighHeight, aMieHeight, aAbsorptionHeight, aAbsorptionFalloff,
            aGroundPrimarySteps, agroundLightSteps);
      }

      float3 col = calculate_atmosphere_scattering(
        ro, rd, max_dist,
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

      OutTexture[DTid.xy] = float4(col, 1.0f);
   } // main
   ```

   ```cpp
   // Common.hlsli
   static float2 get_spherical_uv(float3 rd)
   {
      float2 uv;
      uv.x = atan2(rd.x, rd.z) / (2.0f * PI) + 0.5f;
      uv.y = 0.5f - (asin(clamp(rd.y, -1.0f, 1.0f)) / PI);
      return uv;
   } // get_spherical_uv
   ```

   ```cpp
   // SkyBoxPS.hlsl
   float4 main(PS_INPUT input) : SV_TARGET
  {  
      PS_OUT output;
      float3 rd = normalize(input.localPos);
    
      float3 ro = CAMERA_POSITION; // 카메라 위치
      float2 uv = input.position.xy / float2(SCREEN_RESOLUTION.x, SCREEN_RESOLUTION.y);
      float3 uvw = float3(uv, 1.0f);
    
      float2 skyUV = get_spherical_uv(rd);
      float3 skyColor = SkyLUT.Sample(LinearWrapSampler, skyUV).rgb;

      float2 screenUV = input.position.xy / cScreenResolution;
      float sceneDepth = SceneDepthTexture.Load(int3(input.position.xy, 0)).r;
    
      if (sceneDepth < 1.0f)
      {

      }
      skyColor = 1.0 - exp(-1.0 * skyColor);
      return float4(skyColor, 1.0f);   
   } // main
   ```

</details>

---

## [Assimp](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Assimp): [Animation](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/GPU_Driven_Grass)


<details>
<summary> Summary </summary>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC09.gif?raw=true" width="460" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Animation</strong></p>
</div>


- **Skinned Animation**

   - 버텍스당 가중치를 활용해 GPU에서 병렬로 메쉬를 변형
   
   - 단일 `Draw Call` 로 처리하여 대규모 군중 렌더링에 최적화

- **Rigid Animation (계층적 트랜스폼)** 

   - 계층 구조별로 노드를 분리
   
   - 각 파츠의 고유 행렬을 적용하여 파츠별 개별 제어

</details>

---