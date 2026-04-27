#include "Pch.h"
#include "PostEffects.h"
#include "Data/RenderTexture.h"
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
// define
#define SAMPLER_SLOT        0
#define TEX_RT_SLOT_SRV     0
#define TEX_NOISE_SLOT_SRV  1

using namespace DirectX;
using namespace ShaderHelper;
using namespace SharedConstants;
using namespace PathConstants;

PostEffects::PostEffects() {
    m_resRT = std::make_unique<RenderTexture>();
    m_tempRT = std::make_unique<RenderTexture>();
    m_noiseSRV = nullptr;
} // PostEffects

PostEffects::~PostEffects() {
    m_noiseSRV = nullptr;
} // ~PostEffects

bool PostEffects::Init(const InitParams& params) {
    if (!params.device || !params.hwnd) {
        return false;
    }

    if (!m_resRT->Init(params.device, params.ScreenWidth, params.ScreenHeight,
        RenderTexture::RenderTextureType::Normal, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        return false;
    }

    if (!m_tempRT->Init(params.device, params.ScreenWidth, params.ScreenHeight,
        RenderTexture::RenderTextureType::Normal, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        return false;
    }

    if (!InitVertexShader(params.device, params.hwnd, POST_VS,
        nullptr, 0, m_vertexShader.GetAddressOf(), nullptr)) {
        return false;
    }

    if (!InitPixelShader(params.device, params.hwnd, BLOOM_PS,
        m_bloomShader.GetAddressOf())) {
        return false;
    }
    if (!InitPixelShader(params.device, params.hwnd, LENS_FLARE_PS,
        m_lensFlareShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<LensFlareBuffer>(params.device, m_lensflareBuffer.GetAddressOf())) {
        return false;
    }

    m_noiseSRV = params.noiseSRV;
    return true;
} // Init

void PostEffects::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetSamplers(SAMPLER_SLOT, 1, &params.linerSampler);

    ID3D11ShaderResourceView* nullSRV = nullptr;

    // Bloom (Input -> temp)
    {
        ID3D11RenderTargetView* interRTV = m_tempRT->GetRTV();
        context->OMSetRenderTargets(1, &interRTV, nullptr);

        context->PSSetShader(m_bloomShader.Get(), nullptr, 0);
        context->PSSetShaderResources(TEX_RT_SLOT_SRV, 1, &params.inputSRV);

        context->Draw(3, 0);
        context->PSSetShaderResources(TEX_RT_SLOT_SRV, 1, &nullSRV);
    }

    // Lens Flare (temp -> Result)
    {
        ID3D11RenderTargetView* resRTV = m_resRT->GetRTV();
        context->OMSetRenderTargets(1, &resRTV, nullptr);

        m_lensflareData.sunUV = params.lightUV;
        m_lensflareData.lensMatrix = params.lensMatrix;
        if (!UpdateConstantBuffer(context, m_lensflareBuffer.Get(), m_lensflareData)) {
            return;
        }

        context->PSSetConstantBuffers(2, 1, m_lensflareBuffer.GetAddressOf());
        context->PSSetShader(m_lensFlareShader.Get(), nullptr, 0);

        ID3D11ShaderResourceView* interSRV = m_tempRT->GetSRV();
        context->PSSetShaderResources(TEX_RT_SLOT_SRV, 1, &interSRV);
        context->PSSetShaderResources(TEX_NOISE_SLOT_SRV, 1, &m_noiseSRV);

        context->Draw(3, 0);

        // 정리
        context->PSSetShaderResources(TEX_RT_SLOT_SRV, 1, &nullSRV);
        context->PSSetShaderResources(TEX_NOISE_SLOT_SRV, 1, &nullSRV);
    }
} // Render

void PostEffects::ClearRT(ID3D11DeviceContext* context) {
    m_resRT->Clear(context);
} // ClearRT

ID3D11Texture2D* PostEffects::GetTexture() const {
    return m_resRT->GetTexture();
} // CloudComposite

ID3D11RenderTargetView* PostEffects::GetRTV() const {
    return m_resRT->GetRTV();
} // GetRTV

ID3D11ShaderResourceView* PostEffects::GetSRV() const {
    return m_resRT->GetSRV();
} // GetSRV