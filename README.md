# Sisyphus-Renderer - Animation

![모두](https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp04_%EC%95%A0%EB%8B%88%EB%A9%94%EC%9D%B4%EC%85%98.gif?raw=true)

## 뼈대 기반 변형 애니메이션: Skinned Animation

```cpp
[Skinned Animation 파이프라인]
  │
  ├─ 1. 데이터 파싱 (AssimpLoader)
  │    ├─ 버텍스별 최대 4개의 BoneID와 가중치(Weight) 추출 및 저장
  │    └─ 애니메이션 클립(Keyframes: Pos, Rot, Scale) 및 뼈의 Offset Matrix 매핑
  │
  ├─ 2. 애니메이션 업데이트 (Animator::Update)
  │    ├─ 현재 시간에 맞춰 각 노드의 로컬 트랜스폼 보간 (S * R * T)
  │    ├─ 부모-자식 계층 구조를 순회하며 Global Transform 누적 연산
  │    └─ [핵심] 최종 Bone 행렬 계산: OffsetMatrix * GlobalTransform * InverseRoot
  │
  └─ 3. 렌더링 (SkinnedActor)
       ├─ 매 프레임 업데이트된 256개의 Bone 행렬을 Constant Buffer(BoneBuffer)에 업로드
       ├─ 버텍스 셰이더(VS)로 전달하여 GPU에서 병렬 가중치 연산 수행
       └─ 단 1번의 Draw Call로 전체 캐릭터 메쉬 렌더링 완료
```

- [`SkinnedActor.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Objects/SkinnedActor.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Objects/SkinnedActor.cpp)

- [`Animator.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Components/Animator.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Components/Animator.cpp)

- [`AssimpModel.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Resources/AssimpModel.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Resources/AssimpModel.cpp)

- [`PBRMesh.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Resources/PBRMesh.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Resources/PBRMesh.cpp)

## 계층적 노드 애니메이션: Rigid Animation

```cpp
[Rigid Animation 파이프라인]
  │
  ├─ 1. 데이터 파싱 (AssimpLoader)
  │    ├─ 버텍스에 Bone 데이터가 없는 정적 메쉬들로 로드
  │    └─ Root 노드부터 시작되는 메쉬 부착 지점(Node Hierarchy) 트리 파싱
  │
  ├─ 2. 트랜스폼 캐싱 (Animator::Update)
  │    ├─ Bone 연산을 생략하고, 각 파츠 노드의 Global Transform만 계산
  │    └─ std::unordered_map을 이용해 현재 프레임의 노드별 변환 행렬 캐싱
  │
  └─ 3. 재귀적 렌더링 (DefaultRigid::RenderNode)
       ├─ [루트 노드]부터 시작하여 자식 노드로 재귀 순회
       ├─ 현재 노드의 캐싱된 변환 행렬 * 부모의 World 행렬 연산
       ├─ [핵심] 파츠(메쉬)가 바뀔 때마다 Constant Buffer(WorldBuffer)를 갱신
       └─ 뼈대 버퍼 전송 없이, 각 파츠별로 고유한 World 행렬을 적용하여 Draw Call 수행
```

- [`RigidActor.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Objects/RigidActor.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Objects/RigidActor.cpp)

- [`Animator.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Components/Animator.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Components/Animator.cpp)

- [`AssimpModel.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Resources/AssimpModel.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Animation/src/Graphics/Resources/AssimpModel.cpp)