#include "Pch.h"
#include "Bloom.h"
#include "Data/RenderTexture.h"
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
// define
#define SAMPLER_SLOT  0
#define TEX_SLOT_SRV  0

using namespace DirectX;
using namespace ShaderHelper;

Bloom::Bloom() {
    m_bloomRT = std::make_unique<RenderTexture>();
} // Bloom

Bloom::~Bloom() {
} // ~Bloom

bool Bloom::Init(const InitParams& params) {
    if (!params.device || !params.hwnd) {
        return false;
    }

    if (!m_bloomRT->Init(params.device, params.ScreenWidth, params.ScreenHeight,
        RenderTexture::RenderTextureType::Normal, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        return false;
    }

    if (!InitVertexShader(params.device, params.hwnd, params.vPath,
        nullptr, 0, m_vertexShader.GetAddressOf(), nullptr)) {
        return false;
    }

    if (!InitPixelShader(params.device, params.hwnd, params.pPath,
        m_pixelShader.GetAddressOf())) {
        return false;
    }

    return true;
} // Init

void Bloom::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    context->PSSetSamplers(SAMPLER_SLOT, 1, &params.linerSampler);
    context->PSSetShaderResources(TEX_SLOT_SRV, 1, &params.inputSRV);
    context->Draw(3, 0);
} // Render

void Bloom::ClearRT(ID3D11DeviceContext* context) {
    m_bloomRT->Clear(context);
} // ClearRT

ID3D11Texture2D* Bloom::GetTexture() const {
    return m_bloomRT->GetTexture();
} // CloudComposite

ID3D11RenderTargetView* Bloom::GetRTV() const {
    return m_bloomRT->GetRTV();
} // GetRTV

ID3D11ShaderResourceView* Bloom::GetSRV() const {
    return m_bloomRT->GetSRV();
} // GetSRV