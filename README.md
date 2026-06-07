# Sisyphus-Renderer - Terrain & Grass LOD System
![모두](https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass06_shadow08.gif?raw=true)

<table>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain01_heightmap01.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain01_heightmap02.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain02_%EC%89%90%EB%8F%84%EC%9A%B0%EB%A7%B501.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain02_%EC%89%90%EB%8F%84%EC%9A%B0%EB%A7%B503.png?raw=true" width="320"></td>
  </tr>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass02_%EC%A1%B0%EB%AA%85.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass03_%EB%B2%94%EC%9C%84%EC%A4%84%EC%9E%8402.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass04_InstancedGrass03.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass06_shadow06.png?raw=true" width="320"></td>
  </tr>
</table>

<p align="center">
  <strong> QuadTree Terrain && 3단계 거리 비례 LOD </strong>
</p>

## [파이프라인 구조도]

```cpp
[매 프레임 렌더 단계 - Opaque & Alpha Cutout]
Renderer::MainPass()
  │
  ├─ 1. DrawGround (지형 렌더링)
  │  └─ GroundPS.hlsl 실행
  │     ├─ [입력] GroundTex (t12), Terrain/Object ShadowMap
  │     ├─ HeightMap 기반 정점 높이 및 노말(중앙 차분법) 계산
  │     └─ 원경 처리: smoothstep을 이용해 거리가 멀어질수록 
  │                   Ground 텍스처를 COLOR_DARK_SAND로 부드럽게 블렌딩 (잔디 그림자 효과 대체)
  │
  ├─ 2. DrawGrass - Near (Geometry Shader 동적 빌보드)
  │  └─ GrassGS.hlsl & GrassPS.hlsl 실행 (0 ~ LIMIT_DIST 구간)
  │     ├─ [입력] GrassTex (t0), GrassBuffer (WIND_SPEED 등)
  │     ├─ Point Topology를 입력받아 GS에서 교차된 사각형(Quad) 최대 3개 증식
  │     ├─ GS 내부 2차 LOD: 거리에 따라 생성하는 Quad 개수 조절 (3개 -> 2개 -> 1개)
  │     └─ Sin/Cos 함수를 이용해 Y축 상단부 위주의 자연스러운 바람(Wind) 애니메이션 적용
  │
  └─ 3. DrawGrass - Far (Hardware Instancing 빌보드)
     └─ GrassFarVS.hlsl & GrassFarPS.hlsl 실행 (LIMIT_DIST ~ LIMIT_DIST * 2 구간)
        ├─ [입력] GrassTex (t0), Instance Data (Position, Scale, UV)
        ├─ Y축을 고정한 상태로 항상 카메라를 바라보게 회전 (Fixed Y-Axis Billboard)
        └─ 수작업 MipMap 샘플링: log2(dist/100)를 통해 거리에 따른 알베도 텍스처 밉레벨 조절
```

- [`Ground.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Grass/src/Graphics/Objects/Ground.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Grass/src/Graphics/Objects/Ground.cpp)

- [`Grass.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Grass/src/Graphics/Objects/Grass.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Grass/src/Graphics/Objects/Grass.cpp)

- [`GroundPS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Grass/src/Graphics/HLSL/GroundPS.hlsl)

- [`GrassVS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Grass/src/Graphics/HLSL/GrassVS.hlsl) / [`GrassGS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Grass/src/Graphics/HLSL/GrassGS.hlsl) / [`GrassPS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Grass/src/Graphics/HLSL/GrassPS.hlsl)

- [`GrassFarVS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Grass/src/Graphics/HLSL/GrassFarVS.hlsl) / [`GrassFarPS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Grass/src/Graphics/HLSL/GrassFarPS.hlsl)


## [알면 좋은 것들]

### (1) 3단계 거리 비례 LOD (Level of Detail) 최적화

https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass05_mixGrass02.gif?raw=true

수만 개의 잔디를 모두 3D 폴리곤으로 그리면 심각한 프레임 드랍이 발생

풀과 같은 식생(Vegetation) 렌더링 시 발생하는 **막대한 정점 처리 병목(Vertex Bottleneck)** 을 막기 위해, 카메라와의 거리를 3구간으로 나누어 각기 다른 렌더링 전략을 택함


<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass05_mixGrass01.gif?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass05_mixGrass02.gif?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>지오매트리 - Instanced </strong></p>
</div>


| 거리 구분 | 렌더링 방식 | 특징 |
| :--- | :--- | :--- |
| 0 ~ D | Geometry Shader (Near) | 1~3방향 교차 빌보드 동적 생성, 풍성한 입체감 및 바람 효과 |
| D ~ D×2 | Hardware Instancing (Far) | 정점 생성 없이 인스턴스 버퍼 활용, Y축 고정 카메라 바라보기, MipMap 적용 |
| D×2 ~ | Ground 텍스처 블렌딩 | 폴리곤 렌더링 생략, 바닥 텍스처를 어두운 색상으로 혼합해 잔디밭의 밀도감만 유지 |

### (2) Geometry Shader 내부의 동적 Quad 최적화

CPU에서 무거운 메쉬 데이터를 넘기지 않고, 잔디의 뿌리점(Point) 하나만 입력받아 GPU에서 실시간으로 폴리곤을 만듬

특히 GS 내부에서도 거리에 따라 생성하는 폴리곤 개수를 한 번 더 최적화

```cpp
// GrassGS.hlsl 중거리/근거리 세부 LOD 로직
int billboardCount = 3; 

if (dist > LIMIT_DIST / 2)
    billboardCount = 1; // 꽤 멀면 1장만 (카메라 마주보기)
else if (dist > LIMIT_DIST / 4)
    billboardCount = 2; // 중간 거리면 2장 교차 (X 형태)

[unroll]
for (int d = 0; d < billboardCount; ++d) {
    // 60도(1.0472 rad)씩 회전하며 쿼드 생성
}
```

### (3) Hardware Instancing & 수동 MipMap 샘플링 (Far Grass)

먼 거리의 잔디를 픽셀 셰이더에서 렌더링할 때 발생하는 심각한 외곽선 자글거림(Aliasing)을 방지하기 위해, 픽셀 셰이더 내에서 거리에 비례한 밉맵(MipMap) 레벨을 직접 계산하여 샘플링

```cpp
// GrassFarPS.hlsl
// 거리에 비례하여 0.0 ~ 6.0 사이의 Mip Level 도출
float mipLevel = clamp(log2(input.dist / 100.0f), 0.0f, 6.0f);
float4 col = GrassTex.SampleLevel(LinearSampler, input.uv, mipLevel);

clip(col.a - ALPHA_CUT); // 알파 컷아웃
```

### (4) Ground Shader의 눈속임(Fake) 블렌딩

인스턴싱 잔디조차 사라지는 가장 먼 거리(LIMIT_DIST * 2.0f 이상)에서는 지형 셰이더(`GroundPS.hlsl`)가 그 역할을 대체

```cpp
// GroundPS.hlsl
// LIMIT_DIST의 1.5배 ~ 2.0배 구간에서 서서히 1.0(어두운 흙색)으로 블렌딩
float grassBlend = smoothstep(GROUND_DETAIL_DIST * 2.0f, GROUND_DETAIL_DIST * 1.5f, distToCamera);
float3 baseColor = lerp(groundTex.rgb, COLOR_DARK_SAND, grassBlend);
```

## 참고

- [Rastertek: Terrain Rendering Tutorials](https://rastertek.com/tutterr.html)

- [GPU Gems: Chapter 7. Rendering Countless Blades of Waving Grass](https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-7-rendering-countless-blades-waving-grass)