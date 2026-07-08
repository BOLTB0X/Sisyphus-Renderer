# Sisyphus-Renderer - RenderQueue

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/%EB%A0%8C%EB%8D%94%ED%81%9001.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
</div>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/%EB%A0%8C%EB%8D%94%ED%81%9002.png?raw=true" width="380" style="border:1px solid #ddd; border-radius:4px;" />
</div>


렌더링엔진을 만들면서 느낀게 매번 **DirectX 11 API 상태(State)** 변경하는 것에 번거롭다 느낌 

매 프레임 발생하는 불필요한 *DirectX 11 API 상태(State) 변경* 을 최소화하고, *불투명(Opaque)* 및 *투명(Transparent)* 오브젝트의 정렬을 효율적으로 처리하기 위해 **커스텀 렌더 큐(Render Queue)** 시스템을 도입

## 1. Core Architecture

```cpp
enum class ShaderID : uint16_t {
    Stone = 1,
    StonePillar = 2,
    Arca = 3,
    Tree = 4,
    Rakshasa = 5
}; // ShaderID
```
```cpp
inline uint64_t GenerateSortKey(uint16_t shaderID, uint16_t materialID, float distance) {
    uint32_t depth = static_cast<uint32_t>(distance * 1000.0f);

    uint64_t key = 0;
    key |= (static_cast<uint64_t>(shaderID) << 48);
    key |= (static_cast<uint64_t>(materialID) << 32);
    key |= static_cast<uint64_t>(depth);
    return key;
} // GenerateSortKey
```

```cpp
class RenderQueue {
public:
    struct DrawCommand {
        uint64_t            sortKey;
        ID3D11VertexShader* vs;
        ID3D11PixelShader*  ps;

        std::function<void(ID3D11DeviceContext*)> execute;

        DrawCommand() : sortKey(0), vs(nullptr), ps(nullptr) {
        }
    }; // DrawCommand

public:
    RenderQueue();
    ~RenderQueue();

    void Submit(const DrawCommand& cmd);
    void SortOpaque();
    void SortTransparent();
    void Execute(ID3D11DeviceContext* context);
    void Clear();

private:
    std::vector<DrawCommand> m_commands;
}; // RenderQueue
```

  - [`RenderQueue.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/RenderQueue/src/Core/RenderQueue.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/RenderQueue/src/Core/RenderQueue.cpp)

렌더 큐의 핵심은 상태 동기화 비용을 줄이기 위한 64비트 **정렬 키(Sort Key)** 생성과 **실행 람다(Lambda Context)** 의 캡처

- `64-bit Sort Key` : 셰이더 ID(16비트), 머티리얼 ID(16비트), 카메라와의 거리(32비트)를 하나의 `uint64_t` 키로 결합

- 불투명 오브젝트(SortOpaque): 셰이더 및 머티리얼 변화가 적은 순(오름차순)으로 정렬하여 버텍스/픽셀 셰이더 바인딩 교체 횟수를 줄임

  - [`MayaActor.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/RenderQueue/src/Graphics/Objects/MayaActor.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/RenderQueue/src/Graphics/Objects/MayaActor.cpp)

- 투명 오브젝트(SortTransparent): 블렌딩 연산의 후면-전면(Back-to-Front) 순서를 보장하기 위해 거리 기준 내림차순으로 정렬하여 알파 블렌딩 아티팩트를 방지

    - [`TransparentActor.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/RenderQueue/src/Graphics/Objects/TransparentActor.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/RenderQueue/src/Graphics/Objects/TransparentActor.cpp)

    - [`SkinnedActor.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/RenderQueue/src/Graphics/Objects/SkinnedActor.h) / [`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/RenderQueue/src/Graphics/Objects/SkinnedActor.cpp)

- **Command Pattern (지연 실행)** : 디바이스 컨텍스트를 직접 호출하는 대신, 오브젝트의 상태와 상태 지정을 람다 함수(`std::function<void(ID3D11DeviceContext*)>`) 형태로 캡처하여 `DrawCommand` 구조체에 담아 큐에 `Submit`

## 2. Rendering Pipeline

```
[ 각 Actor 객체 ] ──(Submit)──> [ RenderQueue ] ──(Sort)──> [ State-Minimizing Execution ]
  - MayaActor                      - Opaque: Ascending         - VS/PS State Cache Check
  - SkinnedActor                   - Transparent: Descending   - Batch Draw Call
  - RigidActor / TransparentActor
```

## 3. 객체별 `Submit` 구현

시지프스 엔진 내의 모든 주요 렌더링 컴포넌트는 `RenderQueue::Submit` 구조를 통해 렌더링 파이프라인에 참여

각 객체는 고유한 특성에 맞춰 `DrawCommand` 를 구성

### 1) MayaActor & SkinnedActor

- FBX 및 외부 에셋 기반의 변환(`Transform`) 행렬 및 본 애니메이션(`BoneAnimation`) 데이터를 가짐

- `Submit` 매커니즘

  - 오브젝트가 가진 하위 `Mesh` 개수만큼 루프를 돌며 각 메시의 `MaterialID`와 카메라 사이의 거리를 계산하여 `GenerateSortKey`를 호출

  - 람다 캡처 내부에서 `ConstantBuffer(World 변환 행렬, 뼈대 행렬 버퍼)`를 업데이트하고, 해당 메시가 가진 알베도/노말 등의 `SRV(Shader Resource View)`를 바인딩하는 명령을 캡처하여 제출

### 2) TransparentActor (알파 및 투명 오브젝트)

- 나뭇잎, 풀 등 **알파 테스팅(Alpha Testing) 혹은 알파 블렌딩(Alpha Blending)** 처리가 필요한 컴포넌트

- `Submit` 매커니즘

  - 머티리얼 정보 중 Opacity 슬롯을 필수로 포함하며, SortTransparent 파이프라인으로 분류되기 위해 카메라와의 뎁스 거리를 정밀하게 반영한 키를 빌드

## 참고

- [ChiliTomatoNoodle - Render Queue System [C++ 3D DirectX Tutorial]](https://www.youtube.com/watch?v=yJtyc5b0EHg&t=3)

- [gamedev - d3d11 renderqueue multi pass rendering design best practice](https://gamedev.stackexchange.com/questions/169053/d3d11-renderqueue-multi-pass-rendering-design-best-practice)