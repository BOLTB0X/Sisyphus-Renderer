# Sisyphus-Renderer - Tessellation Terrain

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain05_01.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Tessellation Terrain</strong></p>
</div>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain02_%EC%89%90%EB%8F%84%EC%9A%B0%EB%A7%B501.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain04_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%98%EC%89%90%EB%8F%84%EC%9A%B0%EC%A0%81%EC%9A%A901.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong> QuadTree | Tessellation </strong></p>
</div>

- 카메라 거리별 최적화된 테셀레이션 인자(Tessellation Factor) 계산을 통해 근경의 디테일을 극대화하고 원경의 불필요한 폴리곤 낭비를 제어하며, 텍셀 기반 노말 생성 알고리즘과 높이 기반 멀티 레이어 블렌딩 기법을 도입

<br/>

<table>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain03_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%9801.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain03_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%9802.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain03_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%9803.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain03_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%9804.png?raw=true" width="320"></td>
  </tr>
</table>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/t_%ED%84%B0%EB%A0%88%EC%9D%B8%20%EB%86%92%EC%9D%B4%EC%A1%B0%EC%A0%88.gif?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain04_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%98%EC%89%90%EB%8F%84%EC%9A%B0%EC%A0%81%EC%9A%A902.png?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain04_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%98%EC%89%90%EB%8F%84%EC%9A%B0%EC%A0%81%EC%9A%A903.png?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong> 높이맵 및 광원 위치에 따른 변화 </strong></p>
</div>

cf. [결과 및 시행착오](https://github.com/BOLTB0X/DirectX11-Draw/tree/main/DemoGIF/Renderer/Terrain)


---

## [파이프라인 구조도]

1. **전처리 및 패치 데이터 생성**

   - `Terrain::Init` 단계에서 쿼드(`Quad`) 레이아웃 기반의 정점 및 인덱스 버퍼를 빌드 (32x32 패치 구성)

   - `D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST` 토폴로지를 지정하여 헐 셰이더의 입력 인터페이스 정의

2. **하드웨어 테셀레이션 단계 구동**

    - [`TerrainVS.hlsl1`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Tessellation/src/Graphics/HLSL/TerrainVS.hlsl) -> 제어점 정점 패스스루 연산 수행

    - [`TerrainHS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Tessellation/src/Graphics/HLSL/TerrainHS.hlsl) -> 패치 상수 변환 및 카메라 거리 비례 분할 지수(`CalcTessFactor`) 계산

    - [`TerrainDS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Tessellation/src/Graphics/HLSL/TerrainDS.hlsl) -> 분할된 도메인 좌표 상에서 높이맵 기반 변위 매핑(`Displacement Mapping`) 및 노말 생성

3. **Shading & Shadow**

   - [`TerrainPS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Tessellation/src/Graphics/HLSL/TerrainPS.hlsl) -> 수면 높이, 전이 구역, 눈 한계 축 변수를 기반으로 여러 종의 텍스처 멀티 블렌딩 및 PBR 광원 연산과 포아송 샘플링 섀도우 합성

---

### 1. Terrain 컴포넌트 자원 초기화

```cpp
[Init 단계]
    // 지형 매개변수 설정 및 하드웨어 테셀레이션 파이프라인 버퍼 바인딩
    Terrain::Init()
        └─ 4개 제어점 기반 Quad Patch 메쉬(VertexBuffer / IndexBuffer) 빌드
        └─ VS, HS, DS, PS 각각의 독립 셰이더 컴파일
        └─ 파이프라인 제어용 상수 버퍼(World, TessellationControl, HeightScale, TerrainBlending) 생성
        └─ TextureManager를 통한 고해상도 높이맵 및 레이어별 텍스처(SRV) 바인딩
```

- [`Terrain.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Tessellation/src/Graphics/Objects/Terrain.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Tessellation/src/Graphics/Objects/Terrain.cpp)

- [`PerlinHeightMapCS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/HLSL/PerlinHeightMapCS.hlsl) : ~~GPU Driven Grass 에서 추가~~

---

### 2. 매 프레임 동적 분할 및 제어 셰이딩 파이프라인

```cpp
[매 프레임 렌더 파이프라인]
Renderer::Render()
  └─ Terrain 패스 실행 (RenderQueue 내부 지연 실행 혹은 Direct Render)
        │
        ├─ [파이프라인 상태 설정 및 입력 바인딩]
        │    ├─ IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST)
        │    ├─ HeightMap SRV (VS/DS t0), LinearSampler (s0) 바인딩
        │    └─ 상수 버퍼 가동 (TessellationControlBuffer -> b3, HeightScaleBuffer -> b4)
        │
        ├─ [헐 셰이더 스테이지 - TerrainHS]
        │    ├─ 각 패치의 에지 미드포인트(Edge Midpoint) 좌표 계산
        │    ├─ CalcTessFactor() : CAMERA_POSITION과의 스칼라 거리를 기반으로 거리가 가까울수록 최대 분할율 보간
        │    └─ SV_TessFactor(4개 에지) 및 SV_InsideTessFactor(2개 내부) 결정 및 고정 기능 테셀레이터에 전달
        │
        ├─ [도메인 셰이더 스테이지 - TerrainDS]
        │    ├─ Quad 영역 내의 SV_DomainLocation(적응형 UV좌표) 분할점 생성
        │    ├─ HeightMap.SampleLevel(LinearSampler, finalUV, 0).r * HEIGHT_SCALE 기법으로 로컬 y축 변위 적용
        │    └─ CalculateNormalFromHeightMap() : 인접 4개 방향 텍셀 크기 기반의 수치 미분 노말 벡터 실시간 계산 및 전달
        │
        └─ [픽셀 셰이더 루프 - TerrainPS]
             ├─ 물밑 높이(tWaterLevel), 잔디 한계(tGrassLimit), 만년설 경계(tSnowHeight) 조건 검색
             ├─ 절벽 경사도(N.y의 경사 반비례 축) 파악을 통한 Cliff 텍스처 전이 가중치 획득
             ├─ 독립된 지형 텍스처 시퀀스를 다중 가중치 선형 보간(lerp) 처리 후 알베도/노말 PBR 합성
             └─ calculate_poisson_shadow() 연산을 연계하여 지형 표면의 사실적인 미세 그림자 차폐율 반영
```

## [알면 좋은 것들]

### 패치 경계면 결합 에러(Crack Artifact) 방지를 위한 에지 미드포인트 동기화

#### 1. 원인)
패치 중심점과의 거리만을 기준으로 개별적인 테셀레이터 가중치를 부여하면, 인접한 두 패치의 분할율 계수가 서로 다르게 계산되어 패치와 패치 사이의 경계 정점이 어긋나고 틈새가 벌어지는 크랙(Crack) 현상이 수반됨

#### 2. 해결 방안)

```cpp
PatchTess CalcHSPatchConstants(
    InputPatch<VS_OUT, 4> patch,
    uint patchID : SV_PrimitiveID)
{
    PatchTess pt;

    float3 edgeMid0 = 0.5f * (patch[0].posW + patch[3].posW); // edge0: 0-3
    float3 edgeMid1 = 0.5f * (patch[0].posW + patch[1].posW); // edge1: 0-1
    float3 edgeMid2 = 0.5f * (patch[1].posW + patch[2].posW); // edge2: 1-2
    float3 edgeMid3 = 0.5f * (patch[2].posW + patch[3].posW); // edge3: 2-3

    pt.edgeTess[0] = CalcTessFactor(edgeMid0);
    pt.edgeTess[1] = CalcTessFactor(edgeMid1);
    pt.edgeTess[2] = CalcTessFactor(edgeMid2);
    pt.edgeTess[3] = CalcTessFactor(edgeMid3);

    float3 centerPos = 0.25f * (patch[0].posW + patch[1].posW + patch[2].posW + patch[3].posW);
    float insideTess = CalcTessFactor(centerPos);
    pt.insideTess[0] = insideTess;
    pt.insideTess[1] = insideTess;

    return pt;
} // CalcHSPatchConstants
```

패치 내부 중심점이 아닌, 패치를 이루는 네 에지의 중간 지점(`edgeMid0` ~ `edgeMid3`)을 정밀 추적하고 해당 에지 경계면의 깊이 거리를 기준으로 `SV_TessFactor`를 동기화하는 구조를 채택하여, 인접 패치 간의 분할 밀도가 완벽히 결합하도록 크랙 현상을 사전에 차단함

---

### 고정 데이터가 없는 변위 매핑 지형의 텍셀 수치 미분 노말 생성

#### 1. 원인)

스테틱 메쉬와 달리 도메인 셰이더 단계에서 실시간으로 높이 데이터가 변형(`Displacement`)되므로, DCC 툴에서 미리 베이킹된 버텍스 노말 데이터를 사용하는 방식으로는 지형의 경사면 라이팅을 정확히 연산할 수 없음

#### 2. 해결 방안)

```cpp
float3 CalculateNormalFromHeightMap(float2 uv, float texelSize)
{
    // 상하좌우 높이 샘플링
    float hL = HeightMap.SampleLevel(LinearSampler, uv + float2(-texelSize, 0.0f), 0).r * HEIGHT_SCALE;
    float hR = HeightMap.SampleLevel(LinearSampler, uv + float2(texelSize, 0.0f), 0).r * HEIGHT_SCALE;
    float hB = HeightMap.SampleLevel(LinearSampler, uv + float2(0.0f, -texelSize), 0).r * HEIGHT_SCALE;
    float hT = HeightMap.SampleLevel(LinearSampler, uv + float2(0.0f, texelSize), 0).r * HEIGHT_SCALE;

    // x, z 축 방향의 접선(Tangent) 벡터 생성 후 외적
    float3 tangentX = normalize(float3(2.0f, hR - hL, 0.0f));
    float3 tangentZ = normalize(float3(0.0f, hT - hB, 2.0f));
    
    return normalize(cross(tangentZ, tangentX));
} // CalculateNormalFromHeightMap
```

도메인 셰이더 내에서 도메인 UV 좌표를 기준으로 상, 하, 좌, 우 4방향의 미세 오프셋 텍셀(`texelSize`) 지점 높이를 `SampleLevel`로 바인딩 계산 후, 접선 공간의 기울기 차이를 외적 연산하는 수치 미분 공식(`CalculateNormalFromHeightMap`)을 셰이더 코드을 작성

---

## [최적화 전략]

매 프레임 막대한 단위의 터레인 패치가 분할 기하 연산을 거치므로 오버헤드를 막기 위한 제어 단계 최적화 기법을 사용

### (1) 비선형 거리 감쇠 함수 (Non-linear Distance Attenuation)

```cpp
// TerrainHS.hlsl
// ...
cbuffer TessellationControlBuffer : register(b3)
{
    float  tMinTessDist;
    float  tMaxTessDist;
    float  tMinTessFactor;
    float  tMaxTessFactor;
    
    float4 tPadding;
}; // TessellationControlBuffer

#define MIN_TESS_DISTANCE tMinTessDist
#define MAX_TESS_DISTANCE tMaxTessDist
#define MIN_TESS_FACTOR   tMinTessFactor
#define MAX_TESS_FACTOR   tMaxTessFactor

float CalcTessFactor(float3 patchCenterPos)
{
    float dist = distance(patchCenterPos, CAMERA_POSITION);
    
    float tess = saturate((MAX_TESS_DISTANCE - dist) / (MAX_TESS_DISTANCE - MIN_TESS_DISTANCE));
    
    return lerp(MIN_TESS_FACTOR, MAX_TESS_FACTOR, tess);
} // CalcTessFactor
```

무조건적인 선형 할당을 배제하고 유효 시야 한계점(`MAX_TESS_DISTANCE`)을 초과하는 원경 패치는 최소 분할율(`MIN_TESS_FACTOR = 1.0f`)로 강제 고정하여 불필요한 GPU 기하 버퍼 낭비를 차단

### (2) 섀도우 패스 전용 뎁스 도메인 셰이더 분리

섀도우 맵 생성을 위한 깊이 바인딩Pass에서는 무거운 수치 미분 노말 연산 및 픽셀 셰이더 복잡도가 불필요하므로, 순수 위치 변위와 라이트 행렬 곱셈만 수행하는 간소화된 전용 도메인 셰이더(`TerrainDepthDS.hlsl`)를 구성

- [`TerrainDepthDS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/HLSL/TerrainDepthDS.hlsl)

## 참고

- [Microsoft TechNet: Introduction to Tessellation in Direct3D 11](https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-tessellation)

- [NVIDIA Developer: Dynamic Terrain Tessellation in DX11](https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-2-terrain-rendering-using-gpu-based-geometry)

- [Github - TerrainEngine-OpenGL(fede-vaccaro)](https://github.com/fede-vaccaro/TerrainEngine-OpenGL/tree/master)

- [Rastertek - Tutorial 35: Depth Buffer](https://www.rastertek.com/dx11win10tut35.html)

- [Rastertek - Tutorial 45: Shadow Mapping and Transparency](https://www.rastertek.com/dx11win10tut45.html)

- [Rastertek - Tutorial 49: Hardware Tessellation](https://www.rastertek.com/dx11win10tut49.html)