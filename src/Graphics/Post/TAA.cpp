#include "Pch.h"
#include "TAA.h"
#include "Data/RenderTexture.h"
#include "Resources/ConstantBufferType.h"
// Utils
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
// define
#define SAMPLER_SLOT     0
#define TEX_SLOT_CURRENT 0
#define TEX_SLOT_HISTORY 1
#define CONSTANS_SLOT    2

using namespace DirectX;
using namespace ShaderHelper;
using namespace ConstantBuffer;

TAA::TAA() {
	m_historyRT = std::make_unique<RenderTexture>();
} // TAA

TAA::~TAA() {
} // ~TAA

bool TAA::Init(const InitParams& params) {
	if (!params.device || !params.hwnd) {
		return false;
	}

	if (!m_historyRT->Init(params.device, params.ScreenWidth, params.ScreenHeight,
		RenderTexture::RenderTextureType::Normal, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
		return false;
	}

	if (!InitVertexShader(params.device, params.hwnd, params.vPath,
		nullptr, 0, m_vertexShader.GetAddressOf(), nullptr)) {
		return false;
	}

	if (!InitPixelShader(params.device, params.hwnd, params.pPath, m_pixelShader.GetAddressOf())) {
		return false;
	}

	if (!InitConstantBuffer<TAABuffer>(params.device, m_TAABuffer.GetAddressOf())) {
		return false;
	}

	return true;
} // Init

void TAA::Render(ID3D11DeviceContext* context, const RenderParams& params) {
	m_taaData.blendFactor = params.blendFactor;
	m_taaData.texelSize = params.texelSize;

	if (!UpdateConstantBuffer(context, m_TAABuffer.Get(), m_taaData)) {
		return;
	}

	context->PSSetConstantBuffers(CONSTANS_SLOT, 1, m_TAABuffer.GetAddressOf());
	context->IASetInputLayout(nullptr);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	ID3D11ShaderResourceView* historySRV = m_historyRT->GetSRV();
	
	context->PSSetSamplers(SAMPLER_SLOT, 1, &params.linerSampler);
	context->PSSetShaderResources(TEX_SLOT_CURRENT, 1, &params.currentSRV);
	context->PSSetShaderResources(TEX_SLOT_HISTORY, 1, &historySRV);
	context->Draw(3, 0);
} // Render

void TAA::CopyResource(ID3D11DeviceContext* context, ID3D11Texture2D* current) {
	context->CopyResource(m_historyRT->GetTexture(), current);
} // CopyResource