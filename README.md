# Sisyphus-Renderer (DX11 Real-Time Rendering Engine)

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/%EC%8B%9C%EC%A7%80%ED%94%84%EC%8A%A4%EB%A0%8C%EB%8D%94%EB%9F%AC18.gif?raw=true" width="500" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>🫸🟤 Sisyphus Renderer</strong></p>
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
vcpkg install "imgui[dx11-binding,win32-binding]:x64-windows"
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

## Reference

<details>
<summary> open / close </summary>

###  Volumetric Cloud

- [Guerrilla games: Nubis: Authoring Real-Time Volumetric Cloudscapes with the Decima Engine](https://www.guerrilla-games.com/read/nubis-authoring-real-time-volumetric-cloudscapes-with-the-decima-engine)

- [Patapom: Real-Time Volumetric Rendering](https://patapom.com/topics/Revision2013/Revision%202013%20-%20Real-time%20Volumetric%20Rendering%20Course%20Notes.pdf)

- [Github: RenderEngine(NadirRoGue) - OpenGL](https://github.com/NadirRoGue/RenderEngine/tree/master)

- [Github: TerrainEngine(fede-vaccaro) - OpenGL](https://github.com/fede-vaccaro/TerrainEngine-OpenGL/tree/master)

- [Github: Volumetric Cloud(chihirobelmo) - DX11](https://github.com/chihirobelmo/volumetric-cloud-for-directx11/tree/main)

- [Shadertoy: Himalayas(MdGfzh)](https://www.shadertoy.com/view/MdGfzh)

- [Shadertoy: Enscape Cube(4dSBDt)](https://www.shadertoy.com/view/4dSBDt)

- [Chris' Graphics Blog: Volumetric Rendering Part 1](https://wallisc.github.io/rendering/2020/05/02/Volumetric-Rendering-Part-1.html)

### God Ray

- [Medium: God Rays? What’s that?(Julien Moreau-Mathis)](https://medium.com/community-play-3d/god-rays-whats-that-5a67f26aeac2)

- [Shadertoy: Simple God Rays(ltcXDH)](https://www.shadertoy.com/view/ltcXDH)

- [Shadertoy: Volumetric Light Fog(ssV3zh)](https://www.shadertoy.com/view/ssV3zh)

- [Shadertoy: Lens Flare Example(4sX3Rs)](https://www.shadertoy.com/view/4sX3Rs)

###  Volumetric Fog

- [Shadertoy - volumetric fog - above and below (trapzz)](https://www.shadertoy.com/view/lsc3Ds)

- [Shadertoy - Volumetric fog ride(sergei_am)](https://www.shadertoy.com/view/7s3SRH)

- [Github - VLD3D11 (this project is still in progress)](https://github.com/ACskyline/VLD3D11)

### CPU / GPU Driven Grass

- [NVIDIA Developer - Advancing GPU-Driven Rendering with Work Graphs in Direct3D 12](https://developer.nvidia.com/blog/advancing-gpu-driven-rendering-with-work-graphs-in-direct3d-12/)

- [NVIDIA Developer - Chapter 7. Rendering Countless Blades of Waving Grass](https://developer.nvidia.com/blog/advancing-gpu-driven-rendering-with-work-graphs-in-direct3d-12/)

- [A-programmer's-cave - Grass Shader](https://giordi91.github.io/post/grass/)

- [Roble Programming - GPU로 풀 그리기](https://roble-programing.tistory.com/m/72)

- [Stackoverflow - What is GPU driven rendering?](https://stackoverflow.com/questions/59686151/what-is-gpu-driven-rendering)

- [Rastertek: Terrain Rendering Tutorials](https://rastertek.com/tutterr.html)

- [GPU Gems: Chapter 7. Rendering Countless Blades of Waving Grass](https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-7-rendering-countless-blades-waving-grass)

### Terrain & Tessellation

- [Microsoft TechNet: Introduction to Tessellation in Direct3D 11](https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-tessellation)

- [NVIDIA Developer: Dynamic Terrain Tessellation in DX11](https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-2-terrain-rendering-using-gpu-based-geometry)

- [Github - TerrainEngine-OpenGL(fede-vaccaro)](https://github.com/fede-vaccaro/TerrainEngine-OpenGL/tree/master)

- [Rastertek - Tutorial 35: Depth Buffer](https://www.rastertek.com/dx11win10tut35.html)

- [Rastertek - Tutorial 45: Shadow Mapping and Transparency](https://www.rastertek.com/dx11win10tut45.html)

- [Rastertek - Tutorial 49: Hardware Tessellation](https://www.rastertek.com/dx11win10tut49.html)

### etc

- [ShaderToy : Atmospheric scattering explained - wlBXWK](https://www.shadertoy.com/view/wlBXWK)

- [ChiliTomatoNoodle - Render Queue System [C++ 3D DirectX Tutorial]](https://www.youtube.com/watch?v=yJtyc5b0EHg&t=3)

- [gamedev - d3d11 renderqueue multi pass rendering design best practice](https://gamedev.stackexchange.com/questions/169053/d3d11-renderqueue-multi-pass-rendering-design-best-practice)

- [OpenGL Tutorial 16 - shadow mapping](https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/)

- [Rastertek DX 11 - Tutorial 41: Shadow Mapping](https://www.rastertek.com/dx11win10tut41.html)

- [티스토리 mstone8370 - [DirectX11] 쉐도우 맵과 PCF(Percentage Closer Filtering](https://mstone8370.tistory.com/25)

</details>