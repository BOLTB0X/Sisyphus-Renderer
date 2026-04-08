#include "Pch.h"
#include "Composite.h"
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"

using namespace ShaderHelper;

Composite::Composite() {
} // Composite

Composite::~Composite() {
} // ~Composite

bool Composite::Init(const InitParams& params) {
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

	return true;
} // Init

void Composite::Render(ID3D11DeviceContext* context, ID3D11ShaderResourceView* inputSRV, ID3D11SamplerState* sampler) {
	context->IASetInputLayout(nullptr);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	context->PSSetShaderResources(0, 1, &inputSRV);
	context->PSSetSamplers(0, 1, &sampler);
	// 전체 화면에 사각형을 그리는 드로우 콜
	context->Draw(3, 0); // 삼각형 1개로 전체 화면을 덮는 풀스크린 삼각형
} // Render