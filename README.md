# Sisyphus-Renderer - God Rays(Post-Processing Volumetric Scattering)

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/09Volumetric_%EA%B0%93%EB%A0%88%EC%9D%B402.gif?raw=true" width="650" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>God Rays(Raymarching Light)</strong></p>
</div>

<table>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master01_%EB%82%AE.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master02_%EC%83%88%EB%B2%BD.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master03_%EB%85%B8%EC%9D%84.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master04_%ED%99%A9%ED%98%BC.png?raw=true" width="320"></td>
  </tr>
  <tr>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/t_%EA%B0%93%EB%A0%88%EC%9D%B4.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/t_%EA%B0%93%EB%A0%88%EC%9D%B41.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8406.png?raw=true" width="320"></td>
  <td><img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/t_%EA%B5%AC%EB%A6%84%EB%B0%80%EB%8F%84%EC%A1%B0%EC%A0%88.png?raw=true" width="320"></td>
  </tr>
</table>

<p align="center">
  <strong> Depth & Transmittance 기반 정밀한 차폐(Occlusion) </strong>
</p>

## [파이프라인 구조도]

```cpp
[매 프레임 렌더 단계 - PostProcessing]
PostEffects::RenderGodRays()
  └─ GodRayPS.hlsl 실행 (Radial Blur 기반 스크린 공간 렌더링)
  │
  ├─ [입력]
  │  ├─ InputTex (t0)         - 원본 렌더 타겟 (Scene)
  │  ├─ DepthTex (t1)         - 씬 깊이 버퍼 (오브젝트 차폐 판별용)
  │  ├─ TransmittanceTex (t2) - 구름 투과율 맵 (구름 차폐 판별용)
  │  └─ GodRayBuffer (b2)     - 태양 Screen UV, 밀도, 가중치, 감쇄, 노출값 등
```

- [`D3D11State.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GodRays/src/Graphics/D3D11/D3D11State.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GodRays/src/Graphics/D3D11/D3D11State.cpp)

- [`PostEffects.h`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GodRays/src/Graphics/Post/PostEffects.h)/[`.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GodRays/src/Graphics/Post/PostEffects.cpp)

- [`VolumetricCloudCS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GodRays/src/Graphics/HLSL/VolumetricCloudCS.hlsl)


```cpp
  └─ [픽셀당 실행]
  │
  ├─ 화면 밖 태양 예외 처리 (LIGHT_UV가 화면 밖이면 스킵)
  │
  ├─ 광원 초기화
  │  ├─ get_cross_luminance() : 태양 위치의 기본 밝기 추출
  │  └─ deltaTexCoord = (현재uv - 태양uv) * (1 / NUM_SAMPLES) * DENSITY
  │
  ├─ [Radial Blur 루프 - NUM_SAMPLES(24)회 반복]
  │  │
  │  ├─ uv -= deltaTexCoord (태양 중심을 향해 UV 이동)
  │  │
  │  ├─ 오클루전(Occlusion) 및 마스킹 계산
  │  │  ├─ 구름 투과율(cloudT) 샘플링
  │  │  ├─ 뎁스(Depth) 샘플링 -> 하늘(isSky) 여부 판별
  │  │  └─ cloudT *= isSky (오브젝트에 가려지지 않고, 구름 사이 빈 공간만 통과)
  │  │
  │  ├─ 빛샘(Highlight) 추출
  │  │  ├─ 현재 uv의 InputTex 색상 샘플링
  │  │  └─ 마스킹 적용 (sampleColor *= cloudT)
  │  │
  │  └─ 누적 및 물리적 감쇄 (Decay)
  │     ├─ color += sampleColor * illuminationDecay * WEIGHT
  │     └─ illuminationDecay *= DECAY (거리에 따른 빛 감쇄)
  │
  └─ [최종 색상 출력]
     └─ 태양의 동적 색상 * 누적된 color * EXPOSURE * 태양 기본 밝기
```



<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/09Volumetric_%EA%B0%93%EB%A0%88%EC%9D%B401.gif?raw=true" width="320" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master07_%EA%B0%93%EB%A0%88%EC%9D%B43.gif?raw=true" width="320" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>투과 및 렌즈플레어 혼합</strong></p>
</div>

- [`PostProcess.hlsli`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GodRays/src/Graphics/HLSL/PostProcess.hlsli)

- [`GodRayPS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GodRays/src/Graphics/HLSL/GodRayPS.hlsl)

- [`LensflarePS.hlsl`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/GodRays/src/Graphics/HLSL/LensflarePS.hlsl)

---

## [알면 좋은 것들]

### (1) 십자가 샘플링(Cross Sampling)을 통한 Flickering 방지

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master08_Occlusion.gif?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master_%EA%B5%AC%EB%A6%8407.png?raw=true" width="300" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <p><strong>Flickering 방지 및 광원 위치 확인 </strong></p>
</div>

```cpp
float get_cross_luminance(Texture2D scene, SamplerState samp, float2 lightUV, float InterpolationOffest)
{
    if (lightUV.x < 0.0f || lightUV.x > 1.0f || lightUV.y < 0.0f || lightUV.y > 1.0f)
    {
        return 0.0f;
    }

    float2 offset = float2(0.005f, 0.005f);
    float3 c0 = scene.SampleLevel(samp, lightUV, 0).rgb;
    float3 c1 = scene.SampleLevel(samp, lightUV + float2(offset.x, 0), 0).rgb;
    float3 c2 = scene.SampleLevel(samp, lightUV - float2(offset.x, 0), 0).rgb;
    float3 c3 = scene.SampleLevel(samp, lightUV + float2(0, offset.y), 0).rgb;
    float3 c4 = scene.SampleLevel(samp, lightUV - float2(0, offset.y), 0).rgb;

    float3 avgColor = (c0 + c1 + c2 + c3 + c4) * 0.2f;
    float luminance = dot(avgColor, float3(0.299f, 0.587f, 0.114f)); // 밝기 계산
    
    return smoothstep(InterpolationOffest, 1.0f, luminance);
} // get_cross_luminance
```

- 플리커링 방지

  - *cf. 태양(광원)의 중심 픽셀 1개만 샘플링할 경우, 얇은 오브젝트나 노이즈가 해당 픽셀을 가리면 갓레이 전체가 갑자기 사라지며 깜빡이는 현상이 발생*

- 5-Tap 필터 적용

  - 이를 방지하기 위해 중심과 상하좌우(십자가 모양) 5개의 픽셀을 샘플링하여 평균(`avgColor`)을 내는데, 
  
  - 카메라가 이동할 때 광원의 밝기가 튀는 것을 막아주어 시간적 안정성을 크게 높여줌

- 정밀한 임계값 처리

  - 계산된 평균 밝기에 `smoothstep` 을 적용하여,
  
  - 어설프게 밝은 부분은 무시하고 확실한 광원에서만 빛 산란이 일어나도록 제어

### (2) Depth & Transmittance 기반 정밀한 차폐(Occlusion)

단순히 화면의 밝은 픽셀을 번지게 하는 일반적인 형태의 **Radial Blur**와 달리, 매 스텝마다 `DepthTex`와 `TransmittanceTex` 를 샘플링하여 물리적으로 타당한 차폐를 구현

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/01master09_%ED%88%AC%EA%B3%BC.gif?raw=true" width="320" style="border:1px solid #ddd; border-radius:4px;" />
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/09Volumetric_%EA%B0%93%EB%A0%88%EC%9D%B403.gif?raw=true" width="320" style="border:1px solid #ddd; border-radius:4px;" />
  <br>
  <strong>Depth & Transmittance</strong>
</div>

```cpp
// GodRayPS.hlsl
[loop]
for (int i = 0; i < NUM_SAMPLES; i++)
{
    uv -= deltaTexCoord;
        
    float cloudT = TransmittanceTex.SampleLevel(LinearSampler, uv, 0).r;
        
    float depth = DepthTex.SampleLevel(LinearSampler, uv, 0).r;
    float isSky = (depth <= 0.0001f) ? 1.0f : 0.0f;
    cloudT *= isSky;
        
    float behindCloud = 1.0f - cloudT;
        
    // 밝은 하이라이트(태양 근처) 추출
    float3 sampleColor = InputTex.SampleLevel(LinearSampler, uv, 0).rgb;
        
    sampleColor *= cloudT;
        
    // 누적 및 감쇄
    color += sampleColor * illuminationDecay * WEIGHT;
    illuminationDecay *= DECAY;
} // for
```

- DepthMask (`isSky`)

  지형이나 건물 같은 불투명(Opaque) 오브젝트 뒤에 태양이 숨으면 갓레이 연산이 정확히 차단

- CloudMask (`cloudT`)

  두꺼운 구름 뒤에서는 빛이 통과하지 못하고, 얇은 구름이나 구름 사이의 틈(Transmittance가 높은 곳)에서만 빛이 새어 나오는 진짜 구름 틈새 빛(Crepuscular rays) 연출이 가능

### (3) Screen Space 연산으로 성능 극대화

진짜 3D 공간을 마칭하는 볼류메트릭 클라우드 렌더링과 달리, **God Rays 처리는 이미 완성된 2D 렌더 타겟 위에서 태양 방향을 향해 지정된 횟수(NUM_SAMPLES)만큼만 샘플링하는 Post-Processing Volumetric Scattering 방식을 채택**

<div align="center">
  <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/Volumetric/real/t_%EA%B0%93%EB%A0%88%EC%9D%B43.png?raw=true" width="400" style="border:1px solid #ddd; border-radius:4px;" />
</div>

높은 부하의 3D 볼륨 연산을 피하면서도 AAA급 게임에서 볼 수 있는 극적인 빛 갈라짐 시각 효과를 챙긴 최적화

## 참고

- [Medium: God Rays? What’s that?(Julien Moreau-Mathis)](https://medium.com/community-play-3d/god-rays-whats-that-5a67f26aeac2)

- [Shadertoy: Simple God Rays(ltcXDH)](https://www.shadertoy.com/view/ltcXDH)

- [Shadertoy: Volumetric Light Fog(ssV3zh)](https://www.shadertoy.com/view/ssV3zh)

- [Shadertoy: Lens Flare Example(4sX3Rs)](https://www.shadertoy.com/view/4sX3Rs)

- [Godot Shaders](https://godotshaders.com/shader/)
