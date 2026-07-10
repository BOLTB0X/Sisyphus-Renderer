# Sisyphus-Renderer (DX11 Rendering Engie)

![모두](https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/%EC%98%A4%EC%98%A4%EC%98%A4%EC%98%A4.gif?raw=true)

## Self Introduce

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC17.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC18.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>시지프스 렌더러</strong></p>
</div>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC11.gif?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC12.gif?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC13.gif?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
</div>

## Development Environment

- **IDE** : Visual Studio Community 2022
- **Lang/Graphics API** : C++ 17/ HLSL 5.0 / DirectX 11
- **Library** : `DirectXTK`, `DirectXTex`, `spdlog`, `Assimp`, `ImGui`
- **Build** : CMake 3.21
- **Package Manager** : vcpkg
- **CPU/GPU:** AMD Ryzen 5 3500u Vega Mobile GFX / AMD Radeon Vega 8
- **AI** : Gemini

## Features 

- **Assimp** 로더
- **FBR Shading**
- **Shadow mapping**
- **Atmospheric scattering**
  - *LUT*
  - *CubeMap*
- **Volumetric Cloud**
- **Volumetric Fog**
- **God Rays**
- **Water**
- **SSR**
- **PostProcessing**
  - *Bloom*
  - *YCoCg Variance Clipping*
  - *Composite*
  - *Lensflare*
  - *TAA*
- **Terrain**
  - *HeightMap*
  - *Quad Tree Culling*
  - *Tessellation* 
- **Grass LOD(Level of Detail)** 
  - *CPU Based Geometry + Instancing + Billboard*
  - *GPU Driven Grass*


## Summary

### First. Volumetric

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC01.gif?raw=true" width="550" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Volumetric Cloud 1.0 & God Rays</strong></p>
</div>

<details>
<summary> About Volumetric </summary>

> 언리얼 엔진 실행 불가, 유니티 버벅이는 저사양 노트북 환경 기준

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master.png?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/t_%EA%B0%93%EB%A0%88%EC%9D%B43.png?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>

  | 상황 | FPS |
  |---|---|
  | 일반 구름 뷰 | 50 ~ 59 |
  | 구름층 전체 + GodRays 등 후처리 풀가동 | 39 ~ 49 |

</div>


Web API(Shadertoy) 기반의 유사 구현체들이 브라우저 환경에서 20~30 FPS를 기록하는 것과 대비하면 상당한 차이가 있다 생각함

</details>

---

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9506.gif?raw=true" width="550" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Volumetric Cloud 2.0</strong></p>
</div>

<details>
<summary> About Volumetric 2.0 </summary>

> 정석 Volumetric

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9504.png?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9509.png?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>

  레이마칭 스텝을 줄이기 위해 비정상적으로 누적된 것이 과포화을 하기 위해서 `(현재 거리 - 천장 거리) / (천장 거리 - 바닥 거리)` 을 진행했으나, <br/> 빛 레이마칭 수정으로 수학적으로 올바른 `(현재 거리 - 바닥 거리) / (천장 거리 - 바닥 거리)` 적용
</div>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8409.png?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9508.png?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  수정 전 | 수정 후
</div>

바닥(`Bottom`)에서 0, 천장(`Top`)에서 1로 정상적인 고도 그라데이션이 형성되도록 수정

</details>

---

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC15.gif?raw=true" width="550" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Volumetric Fog</strong></p>
</div>

<details>
<summary> About Volumetric Fog </summary>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B8_%EB%9D%BC%EC%9D%B4%ED%8C%85.gif?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  포그와 갓레이
</div>

**갓레이 보완** 및 Sphere Bounding 구조로 *3D Worley Noise* 와 *Phase Function* 를 결합한 볼류매트릭 포그

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/t_%ED%92%80.png?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B8_%EC%A0%80%EB%85%812.gif?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
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

### Second. CPU / GPU Culling (Terrain, Instanced Foliage)

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC02.gif?raw=true" width="550" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Quadtree Terrain & LOD Grass</strong></p>
</div>

<details>
<summary> About LOD Grass </summary>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass03_%EB%B2%94%EC%9C%84%EC%A4%84%EC%9E%8404.gif?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass02_%EC%A1%B0%EB%AA%85.png?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>

  <p><strong> </strong>HeightMap 적용 Terrain + Grass Level of Detail(Geometry + Instancing, Billboard) </strong></p>

  -Heightmap 기반 지형 생성 (QuadTree + Frustum Culling)
  -이중 Shadow Map 구조: `ObjectShadowMap`, `TerrainShadowMap`
  -Poisson Disk Sampling PCF 소프트 섀도우

  | 거리 구분 | 렌더링 방식 | 특징 |
  | :--- | :--- | :--- |
  | 0 ~ D | Geometry Shader | 3방향 빌보드 적용, 바람 애니메이션 효과 |
  | D ~ D*2 | Hardware Instancing | Y축 고정 빌보드 적용, 밉맵 LOD 사용 |
  | D*2 ~ | Ground 텍스처 블렌딩 | 풀(Grass) 렌더링 없음 (텍스처로 대체) |


</div>

</details>

<br/>

---

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain05_02.gif?raw=true" width="550" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Tessellation Terrain</strong></p>
</div>


<details>
<summary> About Tessellation Terrain </summary>

**CPU 병목 제거** 및 동적 기하 분할을 위해 *Hull / Domain Shader* 와 *HeightMap Displacement* 를 결합한 하드웨어 테셀레이션 지형

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/t_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%98_%EC%99%80%EC%9D%B4%ED%84%B001.png?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/t_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%98_%EC%99%80%EC%9D%B4%ED%84%B002.png?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
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

<br/>

---

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC14.gif?raw=true" width="550" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>GPU Drive Grass / Instanced Foliage </strong></p>
</div>

<details>
<summary> About Instanced Foliage </summary>

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


| 구분 | 렌더링 알고리즘 | 시각적 결과 및 장점 |
| :--- | :--- | :--- |
| **CPU Instancing** | CPU가 배치 연산 후 `DrawIndexedInstanced`를 호출하여 매 프레임 버퍼 전송 | 배치 개수 증가 시 CPU 메모리 복사 및 드로우 콜 병목 심화 |
| **GPU Driven** | 컴퓨트 셰이더가 직접 `StructuredBuffer`를 채우고 GPU가 Indirect로 렌더링 | CPU 개입 없이 수백만 개의 객체 처리, 거리 기반 실시간 컬링으로 대역폭 최적화 |


</details>

<br/>

---


### Third. Water

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC07.gif?raw=true" width="550" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Water</strong></p>
</div>

<details>
<summary> About Water & SSR </summary>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water02_G-Buffer.png?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <br/>
  G 버퍼
</div>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water05_SSR_light02.gif?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
   <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/t_%EB%AC%BC%20%EB%86%92%EC%9D%B4%EC%A1%B0%EC%A0%88.gif?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
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

<br/>

---

### Fourth. Atmospheric scattering

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/%EB%8C%80%EA%B8%B0%EC%82%B0%EB%9E%80.gif?raw=true" width="550" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Atmospheric scattering</strong></p>
</div>

<details>
<summary> About Atmospheric scattering </summary>

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

<br/>

---

### Fifth. Animation

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC09.gif?raw=true" width="550" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Animation</strong></p>
</div>

<details>
<summary> About Animation </summary>

- **Skinned Animation**

   - 버텍스당 가중치를 활용해 GPU에서 병렬로 메쉬를 변형
   
   - 단일 `Draw Call` 로 처리하여 대규모 군중 렌더링에 최적화

- **Rigid Animation (계층적 트랜스폼)** 

   - 계층 구조별로 노드를 분리
   
   - 각 파츠의 고유 행렬을 적용하여 파츠별 개별 제어

</details>

---

<br/>


*cf* [시행착오 및 스크린샷 및 gif 모음](https://github.com/BOLTB0X/DirectX11-Draw/tree/main/DemoGIF/Renderer)

---

## Branches

- [Assimp README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Assimp)

- [Cubemap README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Cubemap)

- [SkyLUT README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/SkyLUT)

- [ShadowMapping README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/ShadowMapping)

- [Animation README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Animation)

- [Volumetric Cloud 1.0 README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud)

- [Volumetric Cloud 2.0 README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud_2.0)

- [Volumetric Fog README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Volumetric_Fog_Light)

- [Water README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Water)

- [GodRays README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/GodRays)

- [Grass README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Grass)

- [Tessellation README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Tessellation)

- [GPU Driven Grass README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/GPU_Driven_Grass)

- [Render Queue README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/RenderQueue)

## Quick Start

<details>
<summary> open / close </summary>

### 1. 필수 요구 사항

- **OS** : Windows 10/11

- **IDE** : [Visual Studio Community 2022](https://visualstudio.microsoft.com/ko/vs/community/) (C++를 사용한 데스크톱 개발 워크로드 포함)

- **Build** : [CMake 3.21](https://cmake.org/cmake/help/latest/release/3.21.html) 이상

- **Package Manager** : [vcpkg](https://vcpkg.io/en/)

### 2. 라이브러리 설치 (`vcpkg`)

```bash
# 터미널/파워쉘에서 다음 라이브러리들을 설치
vcpkg install imgui[directx11-binding,win32-binding]
vcpkg install directxtk
vcpkg install directxtex
vcpkg install spdlog
vcpkg install assimp
```

### 3. 클론 및 빌드

```bash
# 저장소 클론
git clone https://github.com/BOLTB0X/Sisyphus-Renderer.git
cd Sisyphus-Renderer

# 빌드 디렉토리 생성
mkdir build
cd build

# CMake 구성 (vcpkg 경로 설정 필수)
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg 설치 경로]/scripts/buildsystems/vcpkg.cmake

# 프로젝트 열기 (or cmake --build . 실행)
start SisyphusRenderer.sln
```

### 4. 사용한 에셋

- [sketchfab - Madee: Ground//Stone Sphere](https://sketchfab.com/3d-models/groundstone-sphere-1c0f2b2e213348e6a760743a546dc7a6)

- [Calinou: Free blue noise textures](https://github.com/Calinou/free-blue-noise-textures)

- [maximeheckel: noise textures](https://cdn.maximeheckel.com/noises/noise2.png)

- [Learn OpenGL: Tessellation Chapter I: Rendering Terrain using Height Maps](https://learnopengl.com/Guest-Articles/2021/Tessellation/Height-map)

- [sketchfab: Tree GN](https://sketchfab.com/3d-models/tree-gn-40da979cb23f492583ec89c4196cff4e)

- [sketchfab: Stone Pillar](https://sketchfab.com/3d-models/stone-pillar-4b74c340d1bf47ccad35b57deb78b58a)

- [sketchfab: Arca Dwarapala](https://sketchfab.com/3d-models/arca-dwarapala-fe8803efbc0043d7bc3114387c4c1545)

- [rastertek: grass.dds](https://www.rastertek.com/tertut19.html)

- [sketchfab: (Black Myth)Wukong - Di Luo Cha [Animation]](https://sketchfab.com/3d-models/black-myth-wukong-di-luo-cha-animation-79819655a05f497dbb4225726fcb73e3)

</details>

## History

<details>
<summary> open / close </summary>

### [Model Loading(Assimp)](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Assimp)

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Assimp-test.png?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp01_Tree.png?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp02_pillar01.png?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>static Model</strong></p>
</div>

### [Atmospheric Scattering(Cubemap)](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Cubemap)

<div align="center">
<td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/CubeMap/%EB%8C%80%EA%B8%B001_%ED%95%98%EB%8A%9802.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/CubeMap/%EB%8C%80%EA%B8%B002_%EC%9A%B0%EC%A3%BC01.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/CubeMap/%EB%8C%80%EA%B8%B0_%EA%B3%A0%EB%8F%84%EC%97%90%EB%94%B0%EB%A5%B8%EB%B3%80%ED%99%941.gif?raw=true" width="260"></td>
  <br>
  <p><strong>카메라 위치에 따른 동적 베이킹</strong></p>
</div>

### [Atmospheric Scattering(LUT)](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/SkyLUT)

<div align="center">
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT05_new.png?raw=true" width="200"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT06_new.png?raw=true" width="200"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT07_new.png?raw=true" width="200"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT08_new.png?raw=true" width="200"></td>
  <br>
  <p><strong>UAV 로 계산 후 LUT</strong></p>
</div>

### [Shadowmapping](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/ShadowMapping)

<div align="center">
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/ShadowMapping/ShadowMapping01_02PCF-clamp%EC%83%98%ED%94%8C%EB%9F%AC.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain02_%EC%89%90%EB%8F%84%EC%9A%B0%EB%A7%B502.png?raw=true" width="260"></td>
  <br>
  <p><strong>Point | Terrain </strong></p>
</div>

### [Volumetric Cloud 1.0](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud)

<div align="center">
 <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%84.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/08Volumetric_%EB%A0%8C%EC%A6%88%ED%94%8C%EB%A0%88%EC%96%B401.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8409.png?raw=true" width="260"></td>
  <br>
  <p><strong> AAA 급 볼류메트릭 클라우드</strong></p>
</div>

### [God Rays](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/GodRays)

<div align="center">
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master07_%EA%B0%93%EB%A0%88%EC%9D%B4.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master01_%EB%82%AE.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master03_%EB%85%B8%EC%9D%84.png?raw=true" width="260"></td>
    <br>
  <p><strong> Post-Processing Volumetric Scattering</strong></p>
</div>

### [QuadTree / LOD Grass](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Grass)

<div align="center">
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass03_%EB%B2%94%EC%9C%84%EC%A4%84%EC%9E%8403.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass04_InstancedGrass03.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass06_shadow06.png?raw=true" width="260"></td>
    <br>
  <p><strong> Level of Detail(Geometry + Instancing, Billboard) </strong></p>
</div>

### [Animation](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Animation)

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp05_%EC%95%A0%EB%8B%88%EB%A9%94%EC%9D%B4%EC%85%98%EC%89%90%EB%8F%84%EC%9A%B0.gif?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp06_Rigid.gif?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong> Skinned | Rigid </strong></p>
</div>

### [Volumetric Cloud 2.0](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud_2.0)

<div align="center">
 <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9501.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9509.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9508.png?raw=true" width="260"></td>
  <br>
  <p><strong> AAA 급 볼류메트릭 클라우드</strong></p>
</div>

### [Water](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Water)

<div align="center">
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water04_SSR_flow_X04.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/Water05_SSR_light05.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/t_%EB%B0%98%EC%82%AC.png?raw=true" width="260"></td>
    <br>
  <p><strong> Water | SSR </strong></p>
</div>

### [RenderQueue](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/RenderQueue)

<div align="center">
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/%EB%A0%8C%EB%8D%94%ED%81%9001.png?raw=true" width="260"></td>
</div>

### [Tessellation Terrain](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Tessellation)

<div align="center">
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain04_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%98%EC%89%90%EB%8F%84%EC%9A%B0%EC%A0%81%EC%9A%A901.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain03_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%9803.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain03_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%9804.png?raw=true" width="260"></td>
    <br>
  <p><strong> Tessellation Terrain </strong></p>
</div>

### [GPU Drive Grass](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/GPU_Driven_Grass)

<div align="center">
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Grass/GPU_Driven_Grass05.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Grass/GPU_Driven_Grass03.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass08_GPU_Driven_%EC%9A%B0%EB%93%9C06.png?raw=true" width="260"></td>
    <br>
  <p><strong> Instanced Foliage </strong></p>
</div>

### [Volumetric Fog(Light)](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Volumetric_Fog_Light)

<div align="center">
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B8.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B87.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B88.png?raw=true" width="260"></td>
    <br>
  <p><strong> Volumetric Fog </strong></p>
</div>

</details>