# Sisyphus-Renderer - CubeMap(Atmospheric Scattering)

<p align="center">
<table style="width:70%; text-align:center; border-spacing:20px;">
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


## 대기 산란 & 큐브맵 시스템

### [객체 관계도]

- `CubeMap` *(Resource)* : 6개의 면(Face)을 가진 텍스처 배열 관리자, 렌더링을 위한 RTV(Render Target View)와 셰이더에서 읽기 위한 **SRV(Shader Resource View)** 를 동시에 제공

- `Atmosphere` *(Compute/Bake)* : 실제 **거리 기반 대기 산란(Rayleigh & Mie Scattering)** 연산을 담당하며, 6방향의 카메라를 세팅해 **CubeMap에 대기 상태를 구워내는(Baking)** 주체

- `SkyBox` *(Renderer)* : `Atmosphere` 를 소유하며, 씬(Scene)의 배경이 되는 거대한 정육면체 메쉬에 구워진 큐브맵을 매핑하여 렌더링

  ---

### [y축에 따른 산란]

<p align="center">
  <table style="width:100%; text-align:center; border-spacing:20px;">
      <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/CubeMap/%EB%8F%8C%EC%9D%B4%EB%9E%91.png?raw=true" 
             alt="image 2" 
             style="width:300px; height:400px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
      <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/CubeMap/%EC%A2%80%EB%8D%94%EB%86%92%EC%9D%80%ED%95%98%EB%8A%98.png?raw=true" 
             alt="image 2" 
             style="width:300px; height:400px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
      <td style="text-align:center; vertical-align:middle;">
        <p align="center">
        <img src="https://github.com/BOLTB0X/DirectX11-Draw/blob/main/DemoGIF/Renderer/CubeMap/%EB%8C%80%EA%B8%B002_%EC%9A%B0%EC%A3%BC02.png?raw=true" 
             alt="image 2" 
             style="width:300px; height:400px; object-fit:contain; border:1px solid #ddd; border-radius:4px;"/>
        </p>
      </td>
    </tr>
    <tr>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>평지</a>
      </p>
      </td>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>하늘</a>
      </p>
      </td>
      <td style="text-align:center; font-size:14px; font-weight:bold;">
      <p align="center">
      <a></a>우주</a>
      </p>
      </td>
    </tr>
  </table>
</p>


### [[`Atmosphere`/`SkyBox`] 핵심 로직]

#### 1. 6방향 큐브맵 베이킹 ([`Atmosphere.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Cubemap/src/Graphics/Objects/Atmosphere.cpp))

단순히 화면 크기의 **Quad** 에 대기를 그리는 것이 아니라, <br/> 가상의 카메라를 중심에 두고 `+X`, `-X`, `+Y`, `-Y`, `+Z`, `-Z` 6개의 방향을 바라보며 렌더링 결과를 텍스처로 저장

```cpp
void Atmosphere::Bake(ID3D11DeviceContext* context, D3D11State* states, const RenderParams& params) {
    m_targetIdx = (m_activeIdx + 1) % 2;
    m_blendFactor = 0.0f;
    m_isInterpolating = true;

    ID3D11RenderTargetView* pPrevRTV = nullptr;
    ID3D11DepthStencilView* pPrevDSV = nullptr;
    context->OMGetRenderTargets(1, &pPrevRTV, &pPrevDSV);

    // 큐브맵 베이킹을 위한 상태 설정
    context->OMSetDepthStencilState(states->GetDepthNone(), 0);
    context->RSSetState(states->GetCullNone());

    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // 버퍼 업데이트
    UpdateLightBuffer(context, params.diffuse, params.lightDir);
    UpdateAtmosphereBuffer(context);

    // 큐브맵 전용 뷰포트 설정
    D3D11_VIEWPORT cubeVP = { 0.0f, 0.0f,
        (float)ScreenConstants::CUBE_MAP_SIZE, (float)ScreenConstants::CUBE_MAP_SIZE, 
        0.0f, 1.0f };
    context->RSSetViewports(1, &cubeVP);

    for (unsigned int i = 0; i < 6; ++i) {
        PrepareFaceRender(context, i, params); // 면 설정 분리

        m_cubeMaps[m_targetIdx]->RenderBuffer(context);
        context->DrawIndexed(m_cubeMaps[m_targetIdx]->GetIndexCount(), 0, 0);
    }

    context->OMSetRenderTargets(1, &pPrevRTV, pPrevDSV);

    if (pPrevRTV) pPrevRTV->Release();
    if (pPrevDSV) pPrevDSV->Release();
} // Bake
```

```cpp
void Atmosphere::PrepareFaceRender(ID3D11DeviceContext* context, int faceIndex, const RenderParams& params) {
    // 현재 면의 RTV 설정 및 클리어
    float clearColor[4] = { 0, 0, 0, 1 };
    ID3D11RenderTargetView* rtv = m_cubeMaps[m_targetIdx]->GetRTV(faceIndex);
    context->ClearRenderTargetView(rtv, clearColor);
    context->OMSetRenderTargets(1, &rtv, nullptr);

    // 현재 면의 카메라/행렬 버퍼 업데이트
    UpdateMatrixBuffer(context, XMMatrixIdentity(), m_cubeMaps[m_targetIdx]->GetViewMatrix(faceIndex), m_cubeMaps[m_targetIdx]->GetProjMatrix());
    UpdateCameraBuffer(context, params.camPos, m_cubeMaps[m_targetIdx]->GetViewMatrix(faceIndex), m_cubeMaps[m_targetIdx]->GetProjMatrix());
    context->VSSetConstantBuffers(BUFFER_SLOT_MATRIX, 1, m_matrixBuffer.GetAddressOf());
    context->PSSetConstantBuffers(BUFFER_SLOT_LIGHT, 1, m_lightBuffer.GetAddressOf());
    context->PSSetConstantBuffers(BUFFER_SLOT_CAMERA, 1, m_cameraBuffer.GetAddressOf());
    context->PSSetConstantBuffers(BUFFER_SLOT_ATMOSPHERE, 1, m_atmosphereBuffer.GetAddressOf());
} // PrepareFaceRender
```
---

#### 2. 동적 재베이킹 ([`SkyBox.cpp`](https://github.com/BOLTB0X/Sisyphus-Renderer/blob/Cubemap/src/Graphics/Objects/SkyBox.cpp))

카메라가 너무 높이 올라가 대기권에 진입하거나, 태양의 위치(LightDir)가 바뀌면 하늘의 색이 변해야 함<br/>
매 프레임 검사하여 임계치를 넘었을 때만 베이킹을 트리거

```cpp
void SkyBox::IsAtmosphereBakeRequired(ID3D11DeviceContext* context, D3D11State* states , const DirectX::XMFLOAT3& camPos) {
	if (!m_atmosphere) return;

	bool needBake = false;
	if (m_lastBakeCamY == FLT_MAX) {
		//DebugHelper::DebugPrint("첫 Bake 트리거.");
		needBake = true;
	}
	else {
		float dy = fabsf(camPos.y - m_lastBakeCamY);
		if (dy > m_bakeThresholdY) {
			needBake = true;
			//DebugHelper::DebugPrint(fmt::format("Bake 실행 Height 변경: {:.2f} -> {:.2f}", m_lastBakeCamY, camPos.y));
		}
	}
	
	if (needBake) {
		Atmosphere::RenderParams atmoParams;
		float altitudeMultiplier = 500.0f;atmoParams.camPos = camPos;
		atmoParams.camPos.y *= altitudeMultiplier; // 테스트용
		atmoParams.lightDir = { 0.5f, -1.0f, 0.5f };
		atmoParams.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

		m_atmosphere->Bake(context, states, atmoParams);
		m_lastBakeCamY = camPos.y; // 갱신
		//DebugHelper::DebugPrint(fmt::format("[Atmosphere] Auto Bake - 높이: {:.2f}", m_lastBakeCamY));
	}
} // IsAtmosphereBakeRequired
```

---

### [사용 방법: "씬에 던져놓기만 하세요"]

**복잡한 산란 방정식과 큐브맵 생성 로직은 내부에 캡슐화되어 있음**

```cpp
// 1. 초기화
m_SkyBox = std::make_unique<SkyBox>();
m_SkyBox->Init(device, context, hwnd, linearSampler);

// 2. 프레임 업데이트 (재베이킹이 필요한지 스스로 판단)
m_SkyBox->IsAtmosphereBakeRequired(context, states, cameraPos);

// 3. 렌더링
SkyBox::RenderParams skyParams;
skyParams.view = m_Camera->GetViewMatrix();
skyParams.projection = m_Camera->GetProjectionMatrix();
skyParams.lightDir = m_SunDirection;

m_SkyBox->Render(context, skyParams);
```

---

### [알면 좋은 것들]

#### Q1. 왜 매 프레임 그리지 않고 굳이 `CubeMap`에 굽는(Bake) 방식을 선택했나?

**최적화 (Performance) 및 계속 아티팩트 발생**

- 대기 산란은 광선이 대기 입자와 부딪히는 것을 시뮬레이션하기 위해 **무거운 반복문(Raymarching Step)을 픽셀 셰이더에서 수행하는데** <br/> 이를 FHD 해상도(약 200만 픽셀) 전체에 매 프레임 연산하면 프레임이 너무 떨어짐
   
- `1024x1024` 등 고정된 크기의 큐브맵에 가끔씩만 *'구워두고(Baking)'* , 실제 렌더링 때는 텍스처를 단순히 **읽어오기(Sampling)** 만 하므로 GPU 부하를 획기적으로 줄임

   ---

#### Q2. `Atmosphere` 클래스 안에 큐브맵이 2개(`m_cubeMaps[2]`)인 이유는?

환경맵이다 보니 y축이 변경될때 조금 뚝뚝 끊기는 느낌이 있어 여러시도를 해봄

- 핑퐁 블렌딩 (Ping-Pong Blending) : 만약 태양의 위치가 바뀌어 하늘을 새로 구웠을 때, 큐브맵이 1개라면 하늘의 색이 1프레임 만에 '번쩍'하고 튀는 현상(Popping)이 발생

- 2개의 큐브맵을 사용하여, 이전 큐브맵(`Active`)과 방금 새로 구운 큐브맵(`Target`)을 프레임에 걸쳐 서서히 섞어주어(`Lerp`) 시간의 흐름에 따른 부드러운 변화를 얻어보려 함

## 참고

- [ShaderToy : Atmospheric scattering explained - wlBXWK](https://www.shadertoy.com/view/wlBXWK)

- [환경 맵(큐브 맵, Cube Map) 파헤치기 : 네이버 블로그 - sorkelf Blog](https://blog.naver.com/sorkelf/40157348191)

- [[DirectX 11] PBR IBL Specular : 네이버 블로그 - 강동훈](https://m.blog.naver.com/fah204/221589720633)

- [18. SkyBox - 개발 런닝머신 : 티스토리 - dev-treadmill](https://dev-treadmill.tistory.com/118)