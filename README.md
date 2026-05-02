# Sisyphus-Renderer

![모두](https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/%EC%98%A4%EC%98%A4%EC%98%A4%EC%98%A4.gif?raw=true)

<p align="center">
<table style="width:65%; text-align:center; border-spacing:20px;">
<tr>
<td style="text-align:center; vertical-align:middle;">
<p align="center">
<img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master05_%EC%97%AC%EB%AA%852.gif?raw=true"
alt="그가 본 것은"
style="object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
</p>
</td>
</tr>
<tr>
<td style="text-align:center; font-size:14px; font-weight:bold;">
<p align="center">
<a>시지프스의 돌</a>
</p>
</td>
</tr>
</table>
</p>

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
  <table style="width:100%; text-align:center; border-spacing:20px;">
    <tr>
        <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Assimp-test.png?raw=true" 
             alt="image 2" 
             style="width:350px; height:250px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
    </tr>
    <tr>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>Assimp</a>
      </p>
      </td>
    </tr>
  </table>
</p>

## Atmospheric Scattering

### [Cubemap](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/Cubemap)

<p align="center">
  <table style="width:100%; text-align:center; border-spacing:20px;">
    <tr>
        <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/CubeMap/%EB%8C%80%EA%B8%B001_%ED%95%98%EB%8A%9802.png?raw=true" 
             alt="image 2" 
             style="width:350px; height:250px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
             <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/CubeMap/%EB%8C%80%EA%B8%B002_%EC%9A%B0%EC%A3%BC01.png?raw=true" 
             alt="image 2" 
             style="width:350px; height:250px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
      <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/CubeMap/%EB%8C%80%EA%B8%B0_%EA%B3%A0%EB%8F%84%EC%97%90%EB%94%B0%EB%A5%B8%EB%B3%80%ED%99%941.gif?raw=true" 
             alt="image 2" 
             style="width:350px; height:250px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
    </tr>
    <tr>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>CubeMap 1 </a>
      </p>
      </td>
            <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>CubeMap 2 </a>
      </p>
      </td>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>Dynamic CubeMap Baking</a>
      </p>
      </td>
    </tr>
  </table>
</p>

### [LUT](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/SkyLUT/src)

<p align="center">
  <table style="width:100%; text-align:center; border-spacing:20px;">
    <tr>
        <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT05_new.png?raw=true" 
             alt="image 2" 
             style="width:350px; height:250px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
             <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT06_new.png?raw=true" 
             alt="image 2" 
             style="width:350px; height:250px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
      <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT07_new.png?raw=true" 
             alt="image 2" 
             style="width:350px; height:250px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
        <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/SkyBox/SkyLUT08_new.png?raw=true" 
             alt="image 2" 
             style="width:350px; height:250px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
    </tr>
    <tr>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>Sky LUT 1 </a>
      </p>
      </td>
            <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>Sky LUT 2 </a>
      </p>
      </td>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>Sky LUT 3</a>
      </p>
      </td>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>Sky LUT 4</a>
      </p>
      </td>
    </tr>
  </table>
</p>


## [Shadowmapping](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/ShadowMapping)

<p align="center">
  <table style="width:100%; text-align:center; border-spacing:20px;">
    <tr>
        <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/ShadowMapping/ShadowMapping.png?raw=true" 
             alt="image 2" 
             style="width:350px; height:250px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
             <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/ShadowMapping/ShadowMapping01_02PCF-clamp%EC%83%98%ED%94%8C%EB%9F%AC.png?raw=true" 
             alt="image 2" 
             style="width:350px; height:250px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
      <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/ShadowMapping/ShadowMapping01_06%ED%8F%AC%EC%9D%B8%ED%8A%B8.png?raw=true" 
             alt="image 2" 
             style="width:350px; height:250px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
    </tr>
    <tr>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>Shadow </a>
      </p>
      </td>
            <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>PCF</a>
      </p>
      </td>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>Point</a>
      </p>
      </td>
    </tr>
  </table>
</p>

# Volumetric

AAA급 물리 기반 볼류메트릭 레이마칭(구름맵, 월리 노이즈, Raymarch, 다중 산란 근사, Bloom, TAA, God rays)


## [Volumetric Cloud](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud)

<p align="center">
  <table style="width:100%; text-align:center; border-spacing:20px;">
    <tr>
        <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%84.png?raw=true" 
             alt="image 2" 
             style="width:450px; height:350px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
      <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/08Volumetric_%EB%A0%8C%EC%A6%88%ED%94%8C%EB%A0%88%EC%96%B401.png?raw=true" 
             alt="image 2" 
             style="width:450px; height:350px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
          <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8402.gif?raw=true" 
             alt="image 2" 
             style="width:450px; height:350px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
    </tr>
    <tr>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a> 볼류매트릭 </a>
      </p>
      </td>
        <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a> 렌즈플레어 </a>
      </p>
      </td>
        <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a> 태양 위치에 따른 구름 색상 변경 </a>
      </p>
      </td>
    </tr>
  </table>
</p>


## [God Rays](https://github.com/BOLTB0X/Sisyphus-Renderer/tree/VolumetricCloud)

<p align="center">
  <table style="width:100%; text-align:center; border-spacing:20px;">
    <tr>
        <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master06_%EA%B0%93%EB%A0%88%EC%9D%B4.png?raw=true" 
             alt="image 2" 
             style="width:450px; height:350px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
        <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master01_%EB%82%AE.png?raw=true" 
             alt="image 2" 
             style="width:450px; height:350px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
             <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master03_%EB%85%B8%EC%9D%84.png?raw=true" 
             alt="image 2" 
             style="width:450px; height:350px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
    </tr>
    <tr>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a>God Rays </a>
      </p>
      </td>
            <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a> depths </a>
      </p>
      </td>
                  <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a> 렌즈플레어와 혼합 </a>
      </p>
      </td>
    </tr>
  </table>
</p>
