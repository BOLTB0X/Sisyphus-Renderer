# Sisyphus-Renderer

![모두](https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/%EC%98%A4%EC%98%A4%EC%98%A4%EC%98%A4.gif?raw=true)

## Self Introduce

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master05_%EC%97%AC%EB%AA%852.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>시지프스의 돌</strong></p>
</div>

> 언리얼 엔진 실행 불가, 유니티 버벅이는 저사양 노트북 환경 기준

| 상황 | FPS |
|---|---|
| 일반 구름 뷰 | 50 ~ 59 |
| 구름층 전체 + GodRays 등 후처리 풀가동 | 39 ~ 49 |

유사한 ShaderToy 볼류메트릭 구현체들이 동일 환경에서 **20 ~ 30fps** 수준인 것과 비교하면 상당한 차이가 있다 생각함

단순히 빠른 것이 아니라 **품질을 유지하면서** 빠른 것이 핵심

- **Assimp** 로더
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


이 모든 파이프라인이 돌아가면서도 저사양 환경에서 실시간으로 방어되는 프레임이 **이 Sisyphys Renderer 의 자랑**

*cf* [시행착오 및 스크린샷 및 gif 모음](https://github.com/BOLTB0X/DirectX11-Draw/tree/main/DemoGIF/Renderer/Volumetric/real)

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

</details>

## [Assimp - Model Loading](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Assimp)

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Assimp-test.png?raw=true" width="280" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Assimp Loder (병렬 처리)</strong></p>
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
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/ShadowMapping/ShadowMapping.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/ShadowMapping/ShadowMapping01_02PCF-clamp%EC%83%98%ED%94%8C%EB%9F%AC.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/ShadowMapping/ShadowMapping01_06%ED%8F%AC%EC%9D%B8%ED%8A%B8.png?raw=true" width="260"></td>
  <br>
  <p><strong>일반 | PCF | Point</strong></p>
</div>


## Volumetric

### [Volumetric Cloud](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud)

<div align="center">
 <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%84.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/08Volumetric_%EB%A0%8C%EC%A6%88%ED%94%8C%EB%A0%88%EC%96%B401.png?raw=true" width="260"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8402.gif?raw=true" width="260"></td>
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
