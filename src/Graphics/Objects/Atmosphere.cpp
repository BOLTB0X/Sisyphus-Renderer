#include "Pch.h"
#include "Atmosphere.h"
#include "Resources/DefaultMesh.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"

using namespace DirectX;
using namespace SharedConstants;


Atmosphere::Atmosphere() {
	m_mesh = std::make_unique<DefaultMesh>();
    m_transform = Transform();
    m_zenithColor = XMFLOAT4(0.0f, 0.2f, 0.6f, 1.0f);
    m_horizonColor = XMFLOAT4(0.6f, 0.8f, 1.0f, 1.0f);
} // Atmosphere

Atmosphere::~Atmosphere() {
} // ~Atmosphere

bool Atmosphere::Init(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) {
	if (!m_mesh->Init(device, 1, DefaultMesh::DefaultMeshType::Cube))
		return false;

    if (!InitShader(device, hwnd))
		return false;
	return true;
} // Init


void Atmosphere::Render(ID3D11DeviceContext* context, const RenderParams& params) {

    float skyScale = 10.0f;
	SetPosition(params.camPos);
	SetScale(skyScale, skyScale, skyScale);

    if (RenderShader(context, params)) {
        m_mesh->RenderBuffer(context);
        context->DrawIndexed(m_mesh->GetIndexCount(), 0, 0);
    }
    else {
        DebugHelper::DebugPrint("Atmosphere Shader Render 실패!");
    }
} // Render

void Atmosphere::SetPosition(const DirectX::XMFLOAT3& pos) {
    m_transform.SetPosition(pos);
} // SetPosition

void Atmosphere::SetPosition(float x, float y, float z) {
    m_transform.SetPosition(x, y, z);
} // SetPosition

void Atmosphere::SetScale(const DirectX::XMFLOAT3& scale) {
    m_transform.SetScale(scale);
} // SetScale

void Atmosphere::SetScale(float x, float y, float z) {
    m_transform.SetScale(x, y, z);
} // SetScale


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
    buffer.lightDirection = lightDir;

    if (memcmp(&m_prevLightData, &buffer, sizeof(LightBuffer)) == 0) {
        return true;
    }
    if (!UpdateConstantBuffer(context, m_lightBuffer.Get(), buffer)) {
        return false;
    }

    m_prevLightData = buffer;
    return true;
} // UpdateLightBuffer

bool Atmosphere::UpdateAtmosphereBuffer(ID3D11DeviceContext* context,
    const XMFLOAT4& zenith, const XMFLOAT4& horizon) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;

    AtmosphereBuffer buffer;
    buffer.zenithColor = zenith;
    buffer.horizonColor = horizon;

    if (memcmp(&m_prevAtmosphereData, &buffer, sizeof(AtmosphereBuffer)) == 0) {
        return true;
    }
    if (!UpdateConstantBuffer(context, m_atmosphereBuffer.Get(), buffer)) {
        return false;
    }

    m_prevAtmosphereData = buffer;
    return true;
} // UpdateAtmosphereBuffer

bool Atmosphere::RenderShader(ID3D11DeviceContext* context, const RenderParams& params) {
    if (!UpdateMatrixBuffer(context, m_transform.GetWorldMatrix(), params.view, params.projection)) {
        DebugHelper::DebugPrint("Atmosphere: Matrix Buffer 업데이트 실패!");
        return false;
    }
    if (!UpdateLightBuffer(context, params.diffuse, params.lightDir)) {
        DebugHelper::DebugPrint("Atmosphere: Light Buffer 업데이트 실패!");
        return false;
    }

    if (!UpdateAtmosphereBuffer(context, m_zenithColor, m_horizonColor)) {
        DebugHelper::DebugPrint("Atmosphere: Atmosphere Buffer 업데이트 실패!");
        return false;
    }

    context->VSSetConstantBuffers(0, 1, m_matrixBuffer.GetAddressOf());
    context->PSSetConstantBuffers(1, 1, m_lightBuffer.GetAddressOf());
    context->PSSetConstantBuffers(2, 1, m_atmosphereBuffer.GetAddressOf());
    context->IASetInputLayout(m_layout.Get());

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    return true;
} // RenderShader