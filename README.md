# Sisyphus-Renderer - Assimp

<p align="center">
  <table style="width:70%; text-align:center; border-spacing:20px;">
    <tr>
        <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp-%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%8F%8C.gif?raw=true" 
             alt="image 2" 
             style="; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
    </tr>
    <tr>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>Assimp Model Load Test</a>
      </p>
      </td>
    </tr>
  </table>
</p>

## Assimp 사용법

### [객체 관계도]

- `TextureManager` *(Storage)* : `std::shared_ptr` 기반의 전역 텍스처 창고. 동일한 텍스처의 중복 생성을 원천 봉쇄

- `AssimpLoader` *(Worker)* : `TextureManager` 를 주입받아 작동하며, 모델 파일을 해석하고 자동으로 텍스처 검색(Fuzzy Search)을 수행합니다.

- `AssimpModel` *(Base)* : 로더를 관리하고 로드된 메쉬와 머테리얼 데이터를 소유

- `Stone` *(Derived)* : 구체적인 렌더링 로직(Shader 바인딩 등)에만 집중

    ---

### [[AssimpLoader](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Assimp/src/Graphics/Resources/AssimpLoader.cpp) 핵심 로직]

```cpp
bool AssimpLoader::LoadMeshModel(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path, AssimpModel* outModel) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices |
        aiProcess_ConvertToLeftHanded);

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        return false;
    }

    std::string directory = std::filesystem::path(path).parent_path().string();

    ProcessMaterials(scene, device, context, directory, outModel);
    ProcessNode(scene->mRootNode, scene, device, context, outModel);

    return true;
} // LoadMeshModel
```

- [`ProcessMaterials`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Assimp/src/Graphics/Resources/AssimpLoader.cpp#L49)

    - 병렬 처리(`std::future`, `deferredContext`)

    - 명령 녹화 후 메인에서 일괄 실행

    - 자동 매핑(`LoadMaterialElement`)

- [`ProcessNode`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Assimp/src/Graphics/Resources/AssimpLoader.cpp#L32)

    - 루트 노드부터 시작해 재귀적으로 자식 노드를 탐색
    
    - 각 노드가 가진 변환 행렬과 메쉬 인덱스를 처리하여 모델의 계층 구조와 위치를 렌더링에 반영

    ---


### [사용 방법: "경로만 던지세요"]

사용자는 이제 텍스처를 일일이 로드할 필요가 없음

`Stone/textures/` 폴더에 `Stone_basecolor.png` , `Stone_normal.JPEG` 등의 규칙으로 파일만 넣어두면 끝

```cpp
auto texMgr = std::make_shared<TextureManager>();
texMgr->Init(device, context);

// 모델 생성 및 초기화 (Stone이 AssimpModel을 상속받으므로)
auto myStone = std::make_unique<Stone>();
// 경로만 주면 AssimpLoader가 내부적으로 텍스처를 키워드 매칭하여 자동 바인딩함
myStone->Init(device, context, hwnd, texMgr, "assets/Stone/Stone.fbx");

// 렌더링
myStone->Render(context, params);
```

---

### [알면 좋은 것들]

#### Q1. `AssimpModel::Material` 이 텍스처 포인터를 갖는데 `TextureManager` 가 왜 필요한가?

- **메모리 중복 방지** : 100개의 바위 모델이 동일한 *Stone_Albedo.png* 를 사용하더라도 **VRAM에는 단 한 장만 올라가야 하기에**

- **캐싱** : `TextureManager`는 파일 경로를 `Key` 로 사용하여 이미 로드된 리소스는 즉시 반환하고, 없으면 생성하는 **"지능형 창고" 역할을 수행**

#### Q2. 그냥 클래스 안에 로더를 넣으면 안 되나? 꼭 상속해야 하나? (`AssimpModel` 상속의 당위성)

*`AssimpModel` 내부에 로직을 추가해서 사용해도 상관없음, 하지만*

- **데이터와 로직의 분리** : `AssimpModel`은 **메쉬 데이터와 머테리얼 소유권을 관리하는 '데이터 컨테이너' 역할**

- **확장성** : `Stone`, `Tree`, `Player` 등은 각기 다른 셰이더나 상수 버퍼를 가질 수 있기에, 공통된 로딩 로직은 부모에게 맡기고, 자식은 자신만의 고유한 렌더링 스타일에만 집중하기 위해

#### Q3. 파일명이나 경로를 바꾸고 싶다면?

*ex) _basecolor가 아니라 _bc라고 쓰는데..*

- **중앙 관리 시스템** : `SharedConstants::PBRTextureConstants` 내부의 `PBRTEXTURE_KEYWORD_MAP` 만 수정하면 됌

    ```cpp
    // Utils/SharedConstants/PBRTextureConstants.h
    namespace SharedConstants {
    
        namespace PBRTextureConstants {
            enum class PBRTextureType {
                Albedo, Normal, Metallic,
                Roughness, AO, Alpha, 
                Displacement, Emissive, Unknown
            };

            struct PBRTextureKeyword {
                PBRTextureType type;
                std::vector<std::string> keywords;
            };

            inline const    std::vector<PBRTextureKeyword> PBRTEXTURE_KEYWORD_MAP = {
                { PBRTextureType::Albedo,     { "_basecolor", "_albedo", "_alb", "_diffuse", "_col" } },
                { PBRTextureType::Normal,     { "_normal", "_nrm", "_norm", "_n" } },
                { PBRTextureType::Metallic,   { "_metallic", "_metal", "_m" } },
                { PBRTextureType::Roughness,  { "_roughness", "_rough", "_r" } },
                { PBRTextureType::AO,         { "_ao", "_occlusion" } },
                { PBRTextureType::Alpha,      { "_alpha", "_opacity" } }
            }; // PBRTEXTURE_KEYWORD_MAP

        } // PBRTextureConstants
    }
    ```

- 폴더 구조가 바뀌어도 `ProcessMaterials`에서 `pbrDir` 결정 로직이 자동으로 `textures/` 폴더 존재 여부를 체크

#### Q4. 왜 이렇게 복잡하게 구현했나?

*비동기 + COM + 자동 밉맵 구조를 하다보니..*

- 비동기(`std::async`) : 대용량 FBX와 고해상도 텍스처를 로드할 때 메인 스레드(렌더링)가 멈추는 것을 방지하려

- COM(`CoInitializeEx`) : DirectXTex가 사용하는 WIC(이미지 로더)는 Windows COM 기반, 각 작업 스레드는 독립적인 COM 라이브러리 활성화가 필요하기에

- 자동 밉맵 : `DirectX::GenerateMipMaps`를 CPU 스레드에서 미리 수행하여, GPU의 **Immediate Context 경합 없이도 멀리 있는 물체의 자글거림(Aliasing)을 원천 차단**

## 모델 출처

- [sketchfab - Madee: Ground//Stone Sphere](https://sketchfab.com/3d-models/groundstone-sphere-1c0f2b2e213348e6a760743a546dc7a6)