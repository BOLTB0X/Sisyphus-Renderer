# Sisyphus-Renderer

![모두](https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/%EC%98%A4%EC%98%A4%EC%98%A4%EC%98%A4.gif?raw=true)

## Self Introduce

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC5.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC4.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>시지프스 렌더러</strong></p>
</div>

- **Assimp** 로더
- **FBR Shading**
- **Sky LUT**: 대기 산란 Raymarching
- **Shadow mapping** : Poisson Disk Sampling
- **Volumetric Cloud** (CloudMap, Volume Worley Noise)
- **동적 Ambient 전환** : 노을, 낮, 밤
- **3중 Beer's Law** Shadow
- **Depth Probability** (구름 하단 그림자)
- **Bloom + Lens Flare** 후처리
- **God Rays**: 후처리 Volumetric Scattering
- **YCoCg Variance Clipping**
- **TAA**
- **Lensflare**
- **Terrain** : HeightMap, Quad Tree
- **Grass LOD(Level of Detail)** : Geometry + Instancing, Billboard


이 모든 파이프라인이 돌아가면서도 저사양 환경에서 실시간으로 방어되는 프레임이 **이 Sisyphys Renderer 의 자랑**

<br/>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC.gif?raw=true" width="550" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Volumetric Cloud & God Rays</strong></p>
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

<br/>

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


<br/>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC2.gif?raw=true)" width="550" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Terrain & LOD Grass</strong></p>
</div>

<details>
<summary> About LOD Grass </summary>

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass03_%EB%B2%94%EC%9C%84%EC%A4%84%EC%9E%8404.gif?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC3.png?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
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

*cf* [시행착오 및 스크린샷 및 gif 모음](https://github.com/BOLTB0X/DirectX11-Draw/tree/main/DemoGIF/Renderer)

---

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

## Assimp

### [Model Loading](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Assimp)

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Assimp-test.png?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp01_Tree.png?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp02_pillar01.png?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>static Model</strong></p>
</div>

### [Animation](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Animation)

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp05_%EC%95%A0%EB%8B%88%EB%A9%94%EC%9D%B4%EC%85%98%EC%89%90%EB%8F%84%EC%9A%B0.gif?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp06_Rigid.gif?raw=true" width="260" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong> Skinned | Rigid </strong></p>
</div>

## Atmospheric Scattering

### [Cubemap](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Cubemap)

<div align="center">
<td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/CubeMap/%EB%8C%80%EA%B8%B001_%ED%95%98%EB%8A%9802.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/CubeMap/%EB%8C%80%EA%B8%B002_%EC%9A%B0%EC%A3%BC01.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/CubeMap/%EB%8C%80%EA%B8%B0_%EA%B3%A0%EB%8F%84%EC%97%90%EB%94%B0%EB%A5%B8%EB%B3%80%ED%99%941.gif?raw=true" width="260"></td>
  <br>
  <p><strong>카메라 위치에 따른 동적 베이킹</strong></p>
</div>

### [LUT](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/SkyLUT)

<div align="center">
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT05_new.png?raw=true" width="200"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT06_new.png?raw=true" width="200"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT07_new.png?raw=true" width="200"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT08_new.png?raw=true" width="200"></td>
  <br>
  <p><strong>UAV 로 계산 후 LUT</strong></p>
</div>

## [Shadowmapping](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/ShadowMapping)

<div align="center">
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/ShadowMapping/ShadowMapping01_02PCF-clamp%EC%83%98%ED%94%8C%EB%9F%AC.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain02_%EC%89%90%EB%8F%84%EC%9A%B0%EB%A7%B502.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass06_shadow07.png?raw=true" width="260"></td>
  <br>
  <p><strong>Point | Terrain | mix</strong></p>
</div>


## Volumetric

### [Volumetric Cloud(라이팅 과포화)](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud)

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

### [Volumetric Cloud(에너지 보존 법칙)](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud_2.0)

<div align="center">
 <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9501.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9509.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/10Volumetric_2_%EB%A0%88%EC%9D%B4%EB%A7%88%EC%B9%AD%EB%9D%BC%EC%9D%B4%ED%8A%B8%EC%88%98%EC%A0%9508.png?raw=true" width="260"></td>
  <br>
  <p><strong> AAA 급 볼류메트릭 클라우드</strong></p>
</div>

## [LOD Grass](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Grass)

<div align="center">
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass03_%EB%B2%94%EC%9C%84%EC%A4%84%EC%9E%8403.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass04_InstancedGrass03.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass06_shadow06.png?raw=true" width="260"></td>
    <br>
  <p><strong> Level of Detail(Geometry + Instancing, Billboard) </strong></p>
</div>
