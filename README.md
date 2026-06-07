# Sisyphus-Renderer - Assimp

![모두](https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp03_arca.png?raw=true)

<table>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp01_Tree.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp02_pillar01.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp02_pillar02.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp-%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%8F%8C.gif?raw=true" width="320"></td>
  </tr>
</table>


## 멀티스레드 기반 머티리얼 병렬 로딩 파이프라인

```cpp
[AssimpLoader::LoadMeshModel 호출]
  │
  ├─ 1. Assimp로 FBX/OBJ 메타데이터 파싱 (aiScene 생성)
  │
  └─ 2. ProcessMaterials() 실행 (비동기 텍스처 로딩 시작)
       │
       ├─ [메인 스레드] (반복문으로 Material 개수만큼 스레드 생성)
       │   └─ std::async(std::launch::async, ...) 호출 ─┐
       │                                                │
       │  ┌─────────────────────────────────────────────┘
       │  │
       │  ├─ [워커 스레드 1 ~ N] (각 머티리얼 별로 독립 실행)
       │  │   ├─ 1. CoInitializeEx: COM 라이브러리 초기화 (스레드 안전성 확보)
       │  │   ├─ 2. CreateDeferredContext: 해당 스레드 전용 지연 컨텍스트 생성
       │  │   ├─ 3. LoadMaterialElement: Albedo, Normal 등 텍스처 디스크 로드 및 VRAM 할당 명령어 기록
       │  │   │     (이때 Immediate Context를 쓰지 않고 Deferred Context에 작업 내역을 차곡차곡 적어둠)
       │  │   ├─ 4. FinishCommandList: 지금까지 기록한 GPU 명령들을 하나의 캡슐(CommandList)로 포장
       │  │   └─ 5. CoUninitialize: COM 해제 후 결과(Material 데이터 + CommandList) 반환
       │  │
       │  └─ [메인 스레드 대기열] (std::future)
       │
       └─ 3. 결과 수합 및 GPU 명령 실행 (동기화)
           ├─ futures.get() 호출로 모든 워커 스레드의 작업 완료 대기
           ├─ 결과에서 얻은 Material 데이터를 outModel에 추가
           └─ 메인 스레드의 Immediate Context를 통해 ExecuteCommandList() 일괄 호출
              (워커 스레드들이 미리 포장해둔 GPU 명령어들을 메인 스레드에서 한 번에 쏟아냄)
```

- [`AssimpLoader.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Assimp/src/Graphics/Resources/AssimpLoader.h) / [`AssimpLoader.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Assimp/src/Graphics/Resources/AssimpLoader.cpp)

- [`AssimpModel.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Assimp/src/Graphics/Resources/AssimpModel.h) / [`AssimpModel.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Assimp/src/Graphics/Resources/AssimpModel.cpp)
