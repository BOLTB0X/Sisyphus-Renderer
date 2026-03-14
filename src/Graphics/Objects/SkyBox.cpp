#include "Pch.h"
#include "SkyBox.h"
#include "Resources/DefaultMesh.h"
// D3D11
#include "D3D11/D3D11State.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
// define
#define SAMPLER_SLOT 0
#define RSV1_SLOT     0
#define RSV2_SLOT     1

using namespace DirectX;
using namespace SharedConstants;

SkyBox::SkyBox() {
	m_atmosphere = std::make_unique<Atmosphere>();
	m_cubeMesh = std::make_unique<DefaultMesh>();
	m_sampler = nullptr;
	m_lastBakeCamY = FLT_MAX;
} // SkyBox

SkyBox::~SkyBox() {
	m_sampler = nullptr;
} // ~SkyBox

bool SkyBox::Init(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd, ID3D11SamplerState* sampler) {
	if (!m_atmosphere->Init(device, context, hwnd)) {
		return false;
	}
	if (!m_cubeMesh->Init(device, 1, DefaultMesh::DefaultMeshType::Cube)) {
		return false;
	}
	if (!InitShader(device, hwnd)) { 
		return false;
	}

	m_sampler = sampler;
	return true;
} // Init

void SkyBox::Render(ID3D11DeviceContext* context, const RenderParams& params) {
	context->IASetInputLayout(m_layout.Get());
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	context->PSSetSamplers(SAMPLER_SLOT, 1, &m_sampler);

	// 큐브맵 SRV 바인딩
	ID3D11ShaderResourceView* srvCurrent = m_atmosphere->GetCubeMapSRV(m_atmosphere->GetActiveIndex());
	ID3D11ShaderResourceView* srvNext = m_atmosphere->GetCubeMapSRV(m_atmosphere->GetTargetIndex());

	context->PSSetShaderResources(RSV1_SLOT, 1, &srvCurrent);
	context->PSSetShaderResources(RSV2_SLOT, 1, &srvNext);

	Atmosphere::BlendBuffer blendData;
	blendData.blendFactor = m_atmosphere->IsInterpolating() ? m_atmosphere->GetBlendFactor() : 0.0f;

	// Constant Buffer 업데이트 및 바인딩
	UpdateBlendBuffer(context, blendData.blendFactor);
	XMMATRIX view = params.view;
	view.r[3] = XMVectorSet(0, 0, 0, 1);

	float skyScale = 500.0f;
	XMMATRIX world = XMMatrixScaling(skyScale, skyScale, skyScale);

	if (UpdateMatrixBuffer(context, world, view, params.projection)) {
		context->VSSetConstantBuffers(0, 1, m_matrixBuffer.GetAddressOf());
	}

	m_cubeMesh->RenderBuffer(context);
	context->DrawIndexed(m_cubeMesh->GetIndexCount(), 0, 0);

	// 사용 후 SRV 해제
	ID3D11ShaderResourceView* nullSRV = nullptr;
	context->PSSetShaderResources(RSV1_SLOT, 1, &nullSRV);
	context->PSSetShaderResources(RSV2_SLOT, 1, &nullSRV);
} // Render

void SkyBox::Update(float deltaTime) {
	if (m_atmosphere) {
		m_atmosphere->Update(deltaTime);
	}
} // Update

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

void SkyBox::ForceBakeAtmosphere() { m_lastBakeCamY = FLT_MAX; }
Atmosphere* SkyBox::GetAtmosphere() const { return m_atmosphere.get(); }

bool SkyBox::InitShader(ID3D11Device* device, HWND hwnd) {
	using namespace ShaderHelper;
	using namespace ConstantBuffer;

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	if (!InitVertexShader(device, hwnd, PathConstants::SKYBOX_VS,
		layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
		return false;
	}

	if (!InitPixelShader(device, hwnd, PathConstants::SKYBOX_PS, m_pixelShader.GetAddressOf())) {
		return false;
	}

	if (!InitConstantBuffer<MatrixBuffer>(device, m_matrixBuffer.GetAddressOf())) {
		return false;
	}

	if (!InitConstantBuffer<Atmosphere::BlendBuffer>(device, m_blendBuffer.GetAddressOf())) {
		return false;
	}
	return true;
} // InitShader

bool SkyBox::UpdateMatrixBuffer(ID3D11DeviceContext* context,
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

bool SkyBox::UpdateBlendBuffer(ID3D11DeviceContext* context, float blendFactor) {
	using namespace ShaderHelper;
	using namespace ConstantBuffer;

	Atmosphere::BlendBuffer buffer;
	buffer.blendFactor = blendFactor;
	buffer.padding = { 0.0f, 0.0f, 0.0f };

	if (memcmp(&m_prevBlendData, &buffer, sizeof(Atmosphere::BlendBuffer)) == 0) {
		return true;
	}
	if (!UpdateConstantBuffer(context, m_blendBuffer.Get(), buffer)) {
		return false;
	}
	m_prevBlendData = buffer;
	return true;
} // UpdateBlendBuffer