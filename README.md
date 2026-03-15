# Sisyphus-Renderer

![모두](https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/%EC%98%A4%EC%98%A4%EC%98%A4%EC%98%A4.gif?raw=true)

## Quick Start

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

## [Assimp - Model Loading](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Assimp)

<p align="center">
<table style="width:60%; text-align:center; border-spacing:20px;">
<tr>
<td style="text-align:center; vertical-align:middle;">
<p align="center">
<img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Assimp-test.png?raw=true"
alt="Atmosphere Demo"
style="object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
</p>
</td>
</tr>
<tr>
<td style="text-align:center; font-size:14px; font-weight:bold;">
<p align="center">
<a>Atmospheric Scattering & Dynamic CubeMap Baking</a>
</p>
</td>
</tr>
</table>
</p>


- [Assimp](https://github.com/assimp/assimp) 를 통한 Model 로더

- 비동기 + COM + 자동 밉맵 구조 -> 주소만 입력하면 됌

---

## [Cubemap - Atmospheric Scattering](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Cubemap)

<p align="center">
<table style="width:60%; text-align:center; border-spacing:20px;">
<tr>
<td style="text-align:center; vertical-align:middle;">
<p align="center">
<img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/CubeMap/%EB%8C%80%EA%B8%B0_%EA%B3%A0%EB%8F%84%EC%97%90%EB%94%B0%EB%A5%B8%EB%B3%80%ED%99%941.gif?raw=true"
alt="Atmosphere Demo"
style="object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
</p>
</td>
</tr>
<tr>
<td style="text-align:center; font-size:14px; font-weight:bold;">
<p align="center">
<a>Atmospheric Scattering & Dynamic CubeMap Baking</a>
</p>
</td>
</tr>
</table>
</p>

- [Cubemap](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Cubemap/src) 을 통한 대기 산란 표현

- 동적 재베이킹, RayMarching