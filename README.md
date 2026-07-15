# Sisyphus-Renderer (DX11 Rendering Engie)

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC18.gif?raw=true" width="500" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Sisyphus Renderer</strong></p>
</div>

## Self Introduce

> 💡 **전체 파이프라인의 핵심 요약과 주요 Screenshot & GIF 를 [Pipeline & Features Summery README](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/main/src)에서 한눈에 확인하실 수 있습니다.**

<table>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8408.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Volumetric/real/11Volumetric_2_%EC%8B%A4%EB%B2%84%EB%9D%BC%EC%9D%B4%EB%8B%9D03.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Volumetric/real/12Volumetric_2_%EC%B0%A8%ED%8F%90%ED%99%95%EC%9D%B802.png?raw=true" width="320"></td>
  </tr>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/assimp/assimp02_pillar01.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Grass/grass04_InstancedGrass03.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Grass/GPU_Driven_Grass03.png?raw=true" width="320"></td>
  </tr>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Water/t_WaterNormal.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Water/t_%EB%B0%98%EC%82%AC.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Water/Water07_SSR_Shadow03.png?raw=true" width="320"></td>
  <tr>
    <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Terrain/terrain02_%EC%89%90%EB%8F%84%EC%9A%B0%EB%A7%B503.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC03.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Terrain/terrain03_%ED%85%8C%EC%85%80%EB%A0%88%EC%9D%B4%EC%85%9803.png?raw=true" width="320"></td>
  </tr>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Fog/Fog01_01%EB%B0%A4.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B8_%EA%B0%93%EB%A0%88%EC%9D%B4.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/raw/main/DemoGIF/Renderer/Fog/%ED%8F%AC%EA%B7%B89.png?raw=true" width="320"></td>
  </tr>
</table>

<br/>

## Features

- **Assimp** Loader
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

## Branches

> 💡**각 Branch README 에서 상세 설명과 많은 Screenshot & GIF 를 확인하실 수 있습니다.**

| Rendering Feature | Key Optimizations & Logic | Branch Link |
| :--- | :--- | :--- |
| **Volumetric Cloud** | Raymarching 최적화, AAA급 구름 렌더링 | [VolumetricCloud 1.0](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud) / [VolumetricCloud 2.0](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud_2.0) |
| **Volumetric Fog / God Rays** | 포스트 프로세싱 기반 산란 연산 | [Volumetric_Fog](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Volumetric_Fog_Light) / [GodRays](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/GodRays) |
| **Atmospheric Scattering** | 카메라 고도에 따른 동적 베이킹, Sky LUT | [Cubemap](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Cubemap) / [SkyLUT](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/SkyLUT) |
| **Terrain & LOD Grass** | Quad Tree Culling, Tessellation, GPU Driven | [CPU(Quad Tree Culling) Grass](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Grass) / [Tessellation](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Tessellation) / [GPU Driven Grass](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/GPU_Driven_Grass) |
| **Water** | Screen Space Reflection (SSR) 적용 | [Water](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Water) |
| **Shadow Mapping** | PCF 샘플링, Point & Object && Terrain 섀도우 | [ShadowMapping](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/ShadowMapping) |
| **Core Systems** | Render Queue, Assimp 모델 로더, 애니메이션 | [RenderQueue](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/RenderQueue) / [Assimp](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Assimp) / [Animation](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Animation) |

## Development Environment

- **IDE** : Visual Studio Community 2022
- **Lang/Graphics API** : C++ 17/ HLSL 5.0 / DirectX 11
- **Library** : `DirectXTK`, `DirectXTex`, `spdlog`, `Assimp`, `ImGui`
- **Build** : CMake 3.21
- **Package Manager** : vcpkg
- **CPU/GPU:** AMD Ryzen 5 3500u Vega Mobile GFX / AMD Radeon Vega 8

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

---

<br/>

*cf* [시행착오 및 스크린샷 및 gif 모음](https://github.com/BOLTB0X/DirectX11-Draw/tree/main/DemoGIF/Renderer)
