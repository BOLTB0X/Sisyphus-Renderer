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
    m_cubeMap = std::make_unique<CubeMap>();
    m_zenithColor = { 0.0f, 0.2f, 0.6f, 1.0f };
    m_horizonColor = { 0.81f, 0.38f, 0.66f, 1.0f };
} // Atmosphere

Atmosphere::~Atmosphere() {
} // ~Atmosphere

bool Atmosphere::Init(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) {
    m_cubeMap->Init(device, ScreenConstants::CUBE_MAP_SIZE, ScreenConstants::CUBE_MAP_SIZE);
    if (!InitShader(device, hwnd))
		return false;
	return true;
} // Init

void Atmosphere::Bake(ID3D11DeviceContext* context, D3D11State* states, const RenderParams& params) {
    // 큐브맵 베이킹을 위한 상태 설정
    context->OMSetDepthStencilState(states->GetDepthNone(), 0);
    context->RSSetState(states->GetCullNone());

    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // 공통 버퍼 업데이트 (광원, 대기 파라미터는 6면 공통)
    UpdateLightBuffer(context, params.diffuse, params.lightDir);
    UpdateAtmosphereBuffer(context, m_zenithColor, m_horizonColor);

    // 큐브맵 전용 뷰포트 설정
    D3D11_VIEWPORT cubeVP = { 0.0f, 0.0f,
        (float)ScreenConstants::CUBE_MAP_SIZE, (float)ScreenConstants::CUBE_MAP_SIZE, 
        0.0f, 1.0f };
    context->RSSetViewports(1, &cubeVP);

    for (unsigned int i = 0; i < 6; ++i) {
        PrepareFaceRender(context, i, params); // 면 설정 분리

        m_cubeMap->RenderBuffer(context);
        context->DrawIndexed(m_cubeMap->GetIndexCount(), 0, 0);
    }

    // 베이킹 후 RTV 해제
    ID3D11RenderTargetView* nullRTV = nullptr;
    context->OMSetRenderTargets(1, &nullRTV, nullptr);
} // Bake

ID3D11ShaderResourceView* Atmosphere::GetCubeMapSRV() const { return m_cubeMap->GetSRV(); }

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

bool Atmosphere::UpdateAtmosphereBuffer(ID3D11DeviceContext* context,
    const XMFLOAT4& zenith, const XMFLOAT4& horizon) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;
	using namespace SharedConstants::AtmosphereConstants;

    AtmosphereBuffer buffer;
    buffer.zenithColor = zenith;
    buffer.horizonColor = horizon;

    buffer.planetCenter = { 0.0f, -PLANET_RADIUS, 0.0f };
    buffer.planetRadius = PLANET_RADIUS;
    buffer.atmoRadius = ATMOSPHERE_RADIUS;

    buffer.rayleighBeta = RAYLEIGH_SCATTERING_COEFFICIENT;
    buffer.mieBeta = MIE_BETA;
    buffer.absorptionBeta = MIE_SCATTERING_COEFFICIENT;

    buffer.rayleighHeight = RAYLEIGH_HEIGHT;
    buffer.mieHeight = MIE_HEIGHT;
    buffer.absorptionHeight = ABSORPTION_HEIGHT;
    buffer.absorptionFalloff = ABSORPTION_FALLOFF;
    buffer.g = 0.9f;
    buffer.primarySteps = 32;
    buffer.lightSteps = 8;
    buffer.intensity = 40.0f;
	buffer.groundColor = { 0.0f, 0.25f, 0.05f };
	buffer.groundPrimarySteps = 16;
	buffer.groundLightSteps = 4;

    if (memcmp(&m_prevAtmosphereData, &buffer, sizeof(AtmosphereBuffer)) == 0) {
        return true;
    }
    if (!UpdateConstantBuffer(context, m_atmosphereBuffer.Get(), buffer)) {
        return false;
    }

    m_prevAtmosphereData = buffer;
    return true;
} // UpdateAtmosphereBuffer

void Atmosphere::PrepareFaceRender(ID3D11DeviceContext* context, int faceIndex, const RenderParams& params) {
    // 현재 면의 RTV 설정 및 클리어
    float clearColor[4] = { 0, 0, 0, 1 };
    ID3D11RenderTargetView* rtv = m_cubeMap->GetRTV(faceIndex);
    context->ClearRenderTargetView(rtv, clearColor);
    context->OMSetRenderTargets(1, &rtv, nullptr);

    if (faceIndex == 0) { // 6개 면을 다 찍으면 너무 많으니 첫 번째 면만 확인
        DebugPrint(fmt::format("Baking Face 0 - Passing camPos to Shader: ({:.2f}, {:.2f}, {:.2f})",
            params.camPos.x, params.camPos.y, params.camPos.z));
    }

    // 현재 면의 카메라/행렬 버퍼 업데이트
    // 큐브맵의 View/Proj 행렬을 사용해 내부(0,0,0)에서 각 면을 바라보게 함
    UpdateMatrixBuffer(context, XMMatrixIdentity(), m_cubeMap->GetViewMatrix(faceIndex), m_cubeMap->GetProjMatrix());
    //DirectX::XMFLOAT3 bakeCamPos = { 0.0f, 0.0f, 0.0f };
    //UpdateCameraBuffer(context, bakeCamPos, m_cubeMap->GetViewMatrix(faceIndex), m_cubeMap->GetProjMatrix());UpdateCameraBuffer(context, bakeCamPos, m_cubeMap->GetViewMatrix(faceIndex), m_cubeMap->GetProjMatrix());
    UpdateCameraBuffer(context, params.camPos, m_cubeMap->GetViewMatrix(faceIndex), m_cubeMap->GetProjMatrix());
    // 현재 면에 특화된 버퍼 바인딩
    context->VSSetConstantBuffers(BUFFER_SLOT_MATRIX, 1, m_matrixBuffer.GetAddressOf());
    context->PSSetConstantBuffers(BUFFER_SLOT_LIGHT, 1, m_lightBuffer.GetAddressOf());
    context->PSSetConstantBuffers(BUFFER_SLOT_CAMERA, 1, m_cameraBuffer.GetAddressOf());
    context->PSSetConstantBuffers(BUFFER_SLOT_ATMOSPHERE, 1, m_atmosphereBuffer.GetAddressOf());
} // PrepareFaceRender