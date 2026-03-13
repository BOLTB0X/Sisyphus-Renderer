#include "Pch.h"
#include "Atmosphere.h"
#include "Resources/CubeMap.h"
// D3D11
#include "D3D11/D3D11State.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
// define
#define BUFFER_SLOT_MATRIX     0
#define BUFFER_SLOT_LIGHT      1
#define BUFFER_SLOT_CAMERA     2
#define BUFFER_SLOT_ATMOSPHERE 3

using namespace DirectX;
using namespace SharedConstants;
using namespace DebugHelper;

Atmosphere::Atmosphere() {
	m_cubeMaps[0] = std::make_unique<CubeMap>();
	m_cubeMaps[1] = std::make_unique<CubeMap>();
    m_zenithColor = { 0.0f, 0.2f, 0.6f, 1.0f };
    m_horizonColor = { 0.81f, 0.38f, 0.66f, 1.0f };
    m_activeIdx = 0;
    m_targetIdx = 0;
    m_blendFactor = 0.0f;
    m_isInterpolating = false;
    m_prevAtmosphereData.groundColor = { 0.0f, 0.0f, 0.0f };
} // Atmosphere

Atmosphere::~Atmosphere() {
} // ~Atmosphere

bool Atmosphere::Init(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) {
    for (unsigned int i = 0; i < 2; ++i) {
        auto cubeMap = std::make_unique<CubeMap>();
        cubeMap->Init(device, ScreenConstants::CUBE_MAP_SIZE, ScreenConstants::CUBE_MAP_SIZE);
        m_cubeMaps[i] = std::move(cubeMap);
	}
    if (!InitShader(device, hwnd))
		return false;
	return true;
} // Init

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

void Atmosphere::Update(float deltaTime) {
    if (m_isInterpolating) {
        m_blendFactor += deltaTime * 2.0f;
        if (m_blendFactor >= 1.0f) {
            m_blendFactor = 1.0f;
            m_activeIdx = m_targetIdx;
            m_isInterpolating = false;
        }
    }
} // Update

ID3D11ShaderResourceView*     Atmosphere::GetCubeMapSRV(int index) const { return m_cubeMaps[index]->GetSRV(); }
float                         Atmosphere::GetBlendFactor() const { return m_blendFactor; }
int 					      Atmosphere::GetActiveIndex() const { return m_activeIdx; }
int 					      Atmosphere::GetTargetIndex() const { return m_targetIdx; }
bool                          Atmosphere::IsInterpolating() const { return m_isInterpolating; }
Atmosphere::AtmosphereBuffer& Atmosphere::GetAtmosphereBuffer() { return m_atmosphereData; }

bool Atmosphere::InitShader(ID3D11Device* device, HWND hwnd) {
	using namespace ShaderHelper;
    using namespace ConstantBuffer;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (!InitVertexShader(device, hwnd, PathConstants::ATMOSPHERE_VS,
        layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(device, hwnd, PathConstants::ATMOSPHERE_PS, m_pixelShader.GetAddressOf())) {
        return false;
	}

    if (!InitConstantBuffer<MatrixBuffer>(device, m_matrixBuffer.GetAddressOf())
        || !InitConstantBuffer<LightBuffer>(device, m_lightBuffer.GetAddressOf())
        || !InitConstantBuffer<CameraBuffer>(device, m_cameraBuffer.GetAddressOf())
        || !InitConstantBuffer<AtmosphereBuffer>(device, m_atmosphereBuffer.GetAddressOf())) {
        return false;
	}

    return true;
} // InitShader

bool Atmosphere::UpdateMatrixBuffer(ID3D11DeviceContext* context,
    const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;

    MatrixBuffer buffer;
    buffer.world = XMMatrixTranspose(world);
    buffer.view = XMMatrixTranspose(view);
    buffer.projection = XMMatrixTranspose(projection);

    if (memcmp(&m_prevMatrixData, &buffer, sizeof(MatrixBuffer)) == 0) {
        return true;
    }
    if (!UpdateConstantBuffer(context, m_matrixBuffer.Get(), buffer)) {
        return false;
	}

	m_prevMatrixData = buffer;
    return true;
} // UpdateMatrixBuffer

bool Atmosphere::UpdateLightBuffer(ID3D11DeviceContext* context,
    const XMFLOAT4& diffuse, const XMFLOAT3& lightDir) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;

    LightBuffer buffer;
    buffer.diffuseColor = diffuse;
    XMVECTOR ld = XMLoadFloat3(&lightDir);
    float len = XMVectorGetX(XMVector3Length(ld));
    if (len < 1e-6f) {
        XMFLOAT3 fallback = { 0.0f, -1.0f, 0.0f };
        buffer.lightDirection = fallback;
    }
    else {
        ld = XMVector3Normalize(ld);
        XMStoreFloat3(&buffer.lightDirection, ld);
    }
    buffer.padding = 0.0f;

    if (memcmp(&m_prevLightData, &buffer, sizeof(LightBuffer)) == 0) {
        return true;
    }
    if (!UpdateConstantBuffer(context, m_lightBuffer.Get(), buffer)) {
        return false;
    }

    m_prevLightData = buffer;
    return true;
} // UpdateLightBuffer

bool Atmosphere::UpdateCameraBuffer(ID3D11DeviceContext* context, const XMFLOAT3& camPos, const XMMATRIX& view, const XMMATRIX& proj) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;

    CameraBuffer buffer;
    buffer.cameraPosition = camPos;
    buffer.viewInv = XMMatrixTranspose(XMMatrixInverse(nullptr, view));
    buffer.projInv = XMMatrixTranspose(XMMatrixInverse(nullptr, proj));

    if (memcmp(&m_prevCameraData, &buffer, sizeof(CameraBuffer)) == 0) {
        return true;
    }
    if (!UpdateConstantBuffer(context, m_cameraBuffer.Get(), buffer)) {
        return false;
    }
    m_prevCameraData = buffer;
    return true;
} // UpdateCameraBuffer

bool Atmosphere::UpdateAtmosphereBuffer(ID3D11DeviceContext* context) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;
	using namespace SharedConstants::AtmosphereConstants;

    /*m_atmosphereData.zenithColor = { 0.0f, 0.2f, 0.6f, 1.0f };
    m_atmosphereData.horizonColor = { 0.81f, 0.38f, 0.66f, 1.0f };

    m_atmosphereData.planetCenter = { 0.0f, -PLANET_RADIUS, 0.0f };
    m_atmosphereData.planetRadius = PLANET_RADIUS;
    m_atmosphereData.atmoRadius = ATMOSPHERE_RADIUS;

    m_atmosphereData.rayleighBeta = RAYLEIGH_SCATTERING_COEFFICIENT;
    m_atmosphereData.mieBeta = MIE_BETA;
    m_atmosphereData.absorptionBeta = MIE_SCATTERING_COEFFICIENT;

    m_atmosphereData.rayleighHeight = RAYLEIGH_HEIGHT;
    m_atmosphereData.mieHeight = MIE_HEIGHT;
    m_atmosphereData.absorptionHeight = ABSORPTION_HEIGHT;
    m_atmosphereData.absorptionFalloff = ABSORPTION_FALLOFF;
    m_atmosphereData.g = 0.9f;
    m_atmosphereData.primarySteps = 32;
    m_atmosphereData.lightSteps = 8;
    m_atmosphereData.intensity = 40.0f;
    m_atmosphereData.groundColor = { 0.0f, 0.25f, 0.05f };
    m_atmosphereData.groundPrimarySteps = 16;
    m_atmosphereData.groundLightSteps = 4;*/

    if (memcmp(&m_prevAtmosphereData, &m_atmosphereData, sizeof(AtmosphereBuffer)) == 0) {
        return true;
    }
    if (!UpdateConstantBuffer(context, m_atmosphereBuffer.Get(), m_atmosphereData)) {
        return false;
    }

    m_prevAtmosphereData = m_atmosphereData;
    return true;
} // UpdateAtmosphereBuffer

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