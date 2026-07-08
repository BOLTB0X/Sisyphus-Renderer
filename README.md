# Sisyphus-Renderer - GPU Driven Grass & Instancing

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/GPU_Driven_Grass02.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>GPU Driven Grass, 거리별 다중 인스턴싱</strong></p>
</div>

<p align="center">
  하드웨어 카운터 기반 아규먼트 버퍼(DrawInstancedIndirectArgs)를 사용해 동적으로 변화하는 개체 수를 완벽히 추적하며, 카메라 거리에 따라 근경 기하 연산(Near Pass)과 원경 빌보드 연산(Far Pass)을 나누는 거리별 다중 인스턴싱(Multi-Level LOD Instancing) 구조
</p>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/GPU_Driven_Grass05.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/GPU_Driven_Grass03.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong> Grass | Wood </strong></p>
</div>


<table>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass07_GPU_Driven02.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass07_GPU_Driven03.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass07_GPU_Driven04.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass07_GPU_Driven05.png?raw=true" width="320"></td>
  </tr>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass08_GPU_Driven_%EC%9A%B0%EB%93%9C01.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass08_GPU_Driven_%EC%9A%B0%EB%93%9C02.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass08_GPU_Driven_%EC%9A%B0%EB%93%9C03.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass08_GPU_Driven_%EC%9A%B0%EB%93%9C06.png?raw=true" width="320"></td>
  </tr>
</table>

- [결과 및 시행착오](https://github.com/BOLTB0X/DirectX11-Draw/tree/main/DemoGIF/Renderer/Grass)

<br/>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass07_GPU_Driven01.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass08_GPU_Driven_%EC%9A%B0%EB%93%9C07.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
</div>

<br/>

---

## [파이프라인 구조도]

1. **전처리 및 구조화 버퍼 초기화**

   - `GPUGrass::Init` 단계에서 인스턴스 데이터를 담을 대용량 복제 버퍼를 **UAV** 및 **SRV** 바인딩이 가능하도록 생성

   - GPU 내부 카운터를 `Draw` 명령어와 직접 연동하기 위한 `D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS` 플래그 버퍼들 빌드

2. **GPU driven 배치 및 필터링**

   - `PlacementSeedCS.hlsl` -> 의사 난수 해시(`hash_uint2`)와 높이맵 및 지형 노말 데이터

   - 조건 검사(수면 하부 배치 차단, 절벽 경사면 생성 한계 수치, 절두체 컬링 등)를 거쳐 생존한 정점들만 `Near`, `Far`, `Tree` 각각의 `AppendStructuredBuffer`에 실시간 누적

3. **Multi-Category Indirect Draw 실행**

   - `CopyStructureCount` API를 호출하여 각 큐에 쌓인 최종 합격 카운터 값을 아규먼트 버퍼의 `InstanceCount` 슬롯에 동기화

   - 별도의 인풋 레이아웃 결합 없이, 정점 셰이더 단계에서 인스턴스 ID(`SV_InstanceID`)와 버텍스 ID(`SV_VertexID`)를 조합하여 대량의 기하 데이터를 단 한 번의 드로우 콜(`DrawInstancedIndirect`)로 수면 및 지형 위에 셰이딩

---

### 1. GPUGrass 컴포넌트 리소스 초기화

```cpp
[Init 단계]
    // 컴퓨트 배치 자원 및 간접 드로우용 아규먼트 버퍼 구조 정의
    GPUGrass::Init()
        ├─ StructuredBuffer (UAV/SRV, Max: 1,000,000 인스턴스) 생성
        ├─ Args Buffer (D3D11_BIND_UNORDERED_ACCESS, MISC_DRAWINDIRECT_ARGS) 3종 확보
        ├─ PlacementSeedCS 컴퓨트 셰이더 컴파일 및 배치 상수 버퍼 생성
        └─ VS 단계에서 즉석 기하 구조를 생성하기 위한 쿼드 버텍스 정적 바인딩 준비
```

- [`GPUGrass.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/Objects/GPUGrass.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/Objects/GPUGrass.h)

- [`PlacementSeedCS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/Objects/GPUGrass.h)


---

### 2. 매 프레임 GPU Generation 및 간접 드로우 파이프라인

```cpp
[매 프레임 렌더 파이프라인]
Renderer::Render()
  ├─ 1. GPU GenerationPass 실행 (GPUGrass::ComputePlacement)
  │    ├─ CS 단계에 HeightMap(t0), NormalMap(t1) 바인딩
  │    ├─ Near/Far/Tree Append Buffer 각각을 UAV 슬롯(u1, u2, u3)에 마운트
  │    ├─ Dispatch() 구동 -> GPU 내부에서 해시 난수 기반 조건 검사 및 버퍼 적재
  │    └─ Context->CopyStructureCount() 호출하여 각 Buffer의 실시간 개수를 Args 버퍼의 InstanceCount 필드로 직접 복사
  │
  ├─ 2. Near Grass Rendering (GPUGrass::RenderNear)
  │    ├─ 기하 구조 생성을 위한 GS(Geometry Shader) 혹은 확장 인스턴싱 구동
  │    └─ context->DrawInstancedIndirect(m_grassArgsBuffer.Get(), 0)
  │
  ├─ 3. Far Grass Rendering (GPUGrass::RenderFar)
  │    ├─ [PlaceGrassFarVS.hlsl] 구동 -> 카메라 방향을 마주보는 뷰 얼라인 빌보드 쿼드 연산
  │    └─ context->DrawInstancedIndirect(m_grassFarArgsBuffer.Get(), 0)
  │
  └─ 4. Instanced Tree Actor Rendering (GPUGrass::RenderTrees / InstancedVS)
       ├─ [InstancedVS.hlsl] 구동 -> 메시 데이터가 존재하는 고밀도 나무 액터들의 월드 트랜스폼 결합
       └─ context->DrawInstancedIndirect(m_treeArgsBuffer.Get(), 0)
```

- [`PlaceGrassVS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/HLSL/PlaceGrassVS.hlsl)

- [`PlaceGrassFarVS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/HLSL/PlaceGrassFarVS.hlsl)

- [`InstancedVS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/HLSL/InstancedVS.hlsl)

- [`InstancedDepthVS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/HLSL/InstancedDepthVS.hlsl)

- [`InstancedTransparentDepthVS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/HLSL/InstancedTransparentDepthVS.hlsl)

<br/>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass08_GPU_Driven_%EC%9A%B0%EB%93%9C05.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>GPU Driven Grass, 거리별 다중 인스턴싱</strong></p>
</div>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/ShadowMapping/t_DS%EB%B0%8F%EC%9D%B8%EC%8A%A4%ED%84%B4%EC%8B%B1%EC%89%90%EB%8F%84%EC%9A%B0%ED%99%95%EC%9D%B8.gif?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/t_%EC%9A%B0%EB%93%9C.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>DS 쉐도우 맵 | 결과 </strong></p>
</div>

---

## [알면 좋은 것들]

### 1. 하드웨어 카운터 동기화를 통한 CPU-GPU 병목 제거 (`Indirect Arguments`)

#### 원인)

매 프레임 카메라 이동에 따라 컬링되거나 새로 배치되는 식생 오브젝트의 개수를 CPU가 알고 있으려면, GPU 버퍼 데이터를 다시 CPU 메모리로 읽어오는 **Map / Unmap (Readback) 과정이 필수적임**

이 과정에서 파이프라인 동기화 락(Stall)이 걸려 프레임 드랍이 치명적으로 발생함

#### 해결 방안)

CPU는 드로우 콜 명령만 내릴 뿐 개수 확인에 전혀 개입하지 않는 순수 GPU-Driven 구조를 채택

[`PlacementSeedCS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/HLSL/PlacementSeedCS.hlsl)의 `Append/Consume` 카운터 내부 값을 `CopyStructureCount API`를 활용하여, GPU 메모리 내부에서 곧바로 `DrawInstancedIndirectArgs` 버퍼로 직행

```cpp
// GPUGrass.cpp - ComputePlacement() 내부
// CPU의 개입 없이 GPU의 AppendBuffer 카운트를 Indirect Args 버퍼의 InstanceCount 슬롯에 직접 복사
context->CopyStructureCount(m_grassArgsBuffer.Get(), 0, m_grassAppendBuffer->GetUAV());
context->CopyStructureCount(m_grassFarArgsBuffer.Get(), 0, m_grassFarAppendBuffer->GetUAV());
context->CopyStructureCount(m_treeArgsBuffer.Get(), 0, m_treeAppendBuffer->GetUAV());

// 읽어오는 과정 없이 즉시 간접 드로우 실행
context->DrawInstancedIndirect(m_grassArgsBuffer.Get(), 0);
```

### 2. 카메라 벡터 연동형 스크린 공간 빌보드 변환 및 바람 셰이더 애니메이션

#### 1. 원인)

수십만 평방미터의 지형에 고해상도 3D 잔디 메시를 전부 배치하면 기하 파이프라인 과부하로 렌더링이 불가능하며, 고정된 평면 폴리곤을 사용할 경우 측면이나 상단에서 바라볼 때 종이처럼 납작하게 보이는 한계가 존재함

#### 2. 해결 방안)

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/GPU_Driven_Grass04.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
</div>

[`PlaceGrassFarVS`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/HLSL/PlaceGrassFarVS.hlsl) 영역은 4개의 정점 인덱스만으로 **즉석 쿼드를 형성하는 Billboard 공식을 적용**

카메라 포지션과 오브젝트 배치 포지션 사이의 벡터를 외적하여 언제나 카메라를 정면으로 바라보는 회전 행렬을 셰이더 단에서 직접 계산

```cpp
// PlaceGrassFarVS.hlsl - 빌보드 회전 행렬 계산
float3 look = normalize(CAMERA_POSITION - basePos);
float3 right = normalize(cross(float3(0.0f, 1.0f, 0.0f), look));
float3 up = cross(look, right);
```

동시에 전역 시간 변수(`TIME`)와 월드 UV의 위치 성분을 결합한 비선형 삼각함수 노이즈 연산(`sin(TIME * SPEED + pos.x)`)을 버텍스의 상단 영역에 가중치 매핑하여 자연스러운 파도형 바람 흔들림을 구현

```cpp
// PlaceGrassFarVS.hlsl - 월드 좌표 기반 바람 흔들림 (Wind Swaying)
float windOffset = sin(TIME * GRASS_WIND_SPEED + basePos.x * 0.5f + basePos.z * 0.5f) * GRASS_WIND_STRENGTH;
float weight = saturate(v.y); // 상단부일수록 가중치 증가
pos += right * windOffset * weight; // 바람의 영향을 빌보드의 right 벡터 방향으로 적용
```

---

## [최적화 전략]

화면 전반을 채우는 대규모 기하 연산의 부하를 억제하기 위해 정밀한 마스킹 조건 및 드로우 패스 분리를 진행

### (1) 비가시 영역 한계 처리를 통한 하드웨어 레벨 **Culling**

```cpp
// PlaceGrassFarVS.hlsl - 거리 기반 하드웨어 컬링
if (dist < LIMIT_DIST || dist > LIMIT_DIST * 2.0f)
{
    output.position = float4(2.0f, 2.0f, 2.0f, 1.0f); // Clip Space 바깥으로 강제 래핑
    return output;
}
```

컴퓨트 단계의 1차 필터링 이후에도 정점 셰이더 단계에서 거리 경계조건(`LIMIT_DIST`)을 완벽히 이탈한 데이터는 동차 클립 공간(`Homogeneous Clip Space`) 외부 좌표로 강제 변환하여 래스터라이저 단계 이전에 하드웨어 컬링 처리가 유도되도록 유도함

#### (2) 인스턴싱 전용 투명도 차폐 깊이 패스 빌드 [`InstancedTransparentDepthVS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/HLSL/InstancedTransparentDepthVS.hlsl)

Leaf나 풀잎 등 알파 마스킹이 필요한 인스턴스 액터들은 불투명 뎁스 패스에서 심각한 오버드로우(Overdraw) 문제를 유발하므로

픽셀 셰이더 단계 이전에 깊이 테스팅만 가볍게 수행하여 원경 오클루전을 선별하는 전용 셰이더([`InstancedDepthVS`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/HLSL/InstancedDepthVS.hlsl), [`InstancedTransparentDepthVS`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GPU_Driven_Grass/src/Graphics/HLSL/InstancedTransparentDepthVS.hlsl))를 섀도우 맵 및 지형 AO 루프에 연계

```cpp
// InstancedTransparentDepthVS.hlsl - 최소한의 뎁스 캐스팅 레이아웃
PS_DEPTH_INPUT main(VS_INSTANCED_INPUT input)
{
    PS_DEPTH_INPUT output;
    
    // TransformMatrix 생성 (World 행렬)
    matrix transform = matrix(
        input.transform0,
        input.transform1,
        input.transform2,
        input.transform3
    );

    matrix wvp = mul(transform, VIEW_PROJ);
    output.pos = mul(float4(input.pos, 1.0f), wvp);
    
    // 알파 테스팅(clip)을 위한 UV만 전달, 불필요한 노말/탄젠트 연산 완전 제거
    output.uv = input.uv; 

    return output;
}
```

## 참고

- [NVIDIA Developer - Advancing GPU-Driven Rendering with Work Graphs in Direct3D 12](https://developer.nvidia.com/blog/advancing-gpu-driven-rendering-with-work-graphs-in-direct3d-12/)

- [NVIDIA Developer - Chapter 7. Rendering Countless Blades of Waving Grass](https://developer.nvidia.com/blog/advancing-gpu-driven-rendering-with-work-graphs-in-direct3d-12/)

- [A-programmer's-cave - Grass Shader](https://giordi91.github.io/post/grass/)

- [Roble Programming - GPU로 풀 그리기](https://roble-programing.tistory.com/m/72)

- [Stackoverflow - What is GPU driven rendering?](https://stackoverflow.com/questions/59686151/what-is-gpu-driven-rendering)