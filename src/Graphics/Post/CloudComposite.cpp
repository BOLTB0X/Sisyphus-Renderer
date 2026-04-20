#include "Pch.h"
#include "CloudComposite.h"
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
// define
#define SAMPLER_SLOT   0
#define TEX_SLOT_SCENE 0
#define TEX_SLOT_CLOUD 1
#define CONSTANS_SLOT  2

using namespace DirectX;
using namespace ShaderHelper;
using namespace ConstantBuffer;

CloudComposite::CloudComposite() {
	m_prevResolutionData.padding.x = -1.0f;
} // CloudComposite

CloudComposite::~CloudComposite() {
} // ~CloudComposite

bool CloudComposite::Init(const InitParams& params) {
	if (!params.device || !params.hwnd) {
		return false;
	}

	if (!InitVertexShader(params.device, params.hwnd, params.vPath,
		nullptr, 0, m_vertexShader.GetAddressOf(), nullptr)) {
		return false;
	}

	if (!InitPixelShader(params.device, params.hwnd, params.pPath, m_pixelShader.GetAddressOf())) {
		return false;
	}

	if (!InitConstantBuffer<ResolutionBuffer>(params.device, m_resolutionBuffer.GetAddressOf())) {
		return false;
	}

	m_resolutionData.resolution = XMFLOAT2(
		1.0f / params.ScreenWidth,
		1.0f / params.ScreenHeight);

	return true;
} // Init

void CloudComposite::Render(ID3D11DeviceContext* context, const RenderParams& params) {
	if (UpdateResolutionBuffer(context)) {
		context->CSSetConstantBuffers(CONSTANS_SLOT, 1, m_resolutionBuffer.GetAddressOf());
	}

	context->IASetInputLayout(nullptr);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	context->PSSetSamplers(SAMPLER_SLOT, 1, &params.linerSampler);
	context->PSSetShaderResources(TEX_SLOT_SCENE, 1, &params.sceneSRV);
	context->PSSetShaderResources(TEX_SLOT_CLOUD, 1, &params.cloudSRV);

	context->Draw(3, 0); // 삼각형 1개로 전체 화면을 덮는 풀스크린 삼각형
} // Render

bool CloudComposite::UpdateResolutionBuffer(ID3D11DeviceContext* context) {
	using namespace ShaderHelper;

	if (memcmp(&m_prevResolutionData, &m_resolutionData, sizeof(ResolutionBuffer)) == 0) {
		return true;
	}

	if (!UpdateConstantBuffer(context, m_resolutionBuffer.Get(), m_resolutionData)) {
		return false;
	}

	m_prevResolutionData = m_resolutionData;
	return true;
} // UpdateResolutionBuffer