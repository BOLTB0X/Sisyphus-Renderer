#include "Pch.h"
#include "PostEffects.h"
#include "Data/RenderTexture.h"
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
// define
#define SAMPLER_SLOT         0
#define TEX_RT_SLOT_SRV      0
#define TEX_C_FLARE_SLOT_SRV 0
#define TEX_NOISE_SLOT_SRV   1
#define TEX_C_RAY_SLOT_SRV   1
#define TEX_COMPO_SLOT_SRV   2
#define TEX_DEPTH_SLOT_SRV   2
#define TEX_CLOUD_SLOT_SRV   3
#define TEX_RAY_SLOT_SRV     4
#define CB_LENSFLARE_SLOT    2
#define CB_GOD_RAY_SLOT      2

using namespace DirectX;
using namespace ShaderHelper;
using namespace SharedConstants;
using namespace PathConstants;

PostEffects::PostEffects() {
    m_resRT = std::make_unique<RenderTexture>();
    m_bloomRT = std::make_unique<RenderTexture>();
    m_rayRT = std::make_unique<RenderTexture>();
    m_flareRT = std::make_unique<RenderTexture>();
    m_noiseSRV = nullptr;
    m_depthSRV = nullptr;
} // PostEffects

PostEffects::~PostEffects() {
    m_noiseSRV = nullptr;
    m_depthSRV = nullptr;
} // ~PostEffects

bool PostEffects::Init(const InitParams& params) {
    if (!params.device || !params.hwnd) {
        return false;
    }

    if (!m_resRT->Init(params.device, params.ScreenWidth, params.ScreenHeight,
        RenderTexture::RenderTextureType::Normal, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        return false;
    }

    if (!m_bloomRT->Init(params.device, params.ScreenWidth, params.ScreenHeight,
        RenderTexture::RenderTextureType::Normal, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        return false;
    }

    if (!m_rayRT->Init(params.device, params.ScreenWidth, params.ScreenHeight,
        RenderTexture::RenderTextureType::Normal, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        return false;
    }

    if (!m_flareRT->Init(params.device, params.ScreenWidth, params.ScreenHeight,
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

    if (!InitPixelShader(params.device, params.hwnd, GOD_RAY_PS,
        m_godRayShader.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(params.device, params.hwnd, LENS_FLARE_PS,
        m_lensFlareShader.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(params.device, params.hwnd, COMPOSITE_PS,
        m_compositeShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<GodRayBuffer>(params.device, m_godRayBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<LensFlareBuffer>(params.device, m_lensflareBuffer.GetAddressOf())) {
        return false;
    }

    m_noiseSRV = params.noiseSRV;
    m_depthSRV = params.depthSRV;
    return true;
} // Init

void PostEffects::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetSamplers(SAMPLER_SLOT, 1, &params.linerSampler);

    ApplyBloom(context, params.inputSRV);
    ApplyGodRays(context, params.inputSRV, params.transmittanceSRV, params.lightUV);
    ApplyLensflare(context, params.inputSRV, params.cloudSRV, params.lightUV);
    ApplyComposite(context);
} // Render

void PostEffects::ApplyBloom(ID3D11DeviceContext* context, ID3D11ShaderResourceView* sceneSRV) {
    ID3D11ShaderResourceView* nullSRV = nullptr;

    ID3D11RenderTargetView* interRTV = m_bloomRT->GetRTV();
    context->OMSetRenderTargets(1, &interRTV, nullptr);

    context->PSSetShader(m_bloomShader.Get(), nullptr, 0);
    context->PSSetShaderResources(TEX_RT_SLOT_SRV, 1, &sceneSRV);

    context->Draw(3, 0);
    context->PSSetShaderResources(TEX_RT_SLOT_SRV, 1, &nullSRV);
} // ApplyBloom

void PostEffects::ApplyGodRays(ID3D11DeviceContext* context,
    ID3D11ShaderResourceView* sceneSRV, ID3D11ShaderResourceView* trSRV, DirectX::XMFLOAT2 lightUV) {
    ID3D11ShaderResourceView* nullSRV = nullptr;

    ID3D11RenderTargetView* godRayRTV = m_rayRT->GetRTV();
    context->OMSetRenderTargets(1, &godRayRTV, nullptr);

    m_godRayData.lightUV = lightUV;
    if (!UpdateConstantBuffer(context, m_godRayBuffer.Get(), m_godRayData)) {
        return;
    }

    context->PSSetConstantBuffers(CB_GOD_RAY_SLOT, 1, m_godRayBuffer.GetAddressOf());
    context->PSSetShader(m_godRayShader.Get(), nullptr, 0);

    context->PSSetShaderResources(TEX_RT_SLOT_SRV, 1, &sceneSRV);
    context->PSSetShaderResources(TEX_DEPTH_SLOT_SRV, 1, &m_depthSRV);
    context->PSSetShaderResources(TEX_CLOUD_SLOT_SRV, 1, &trSRV);
    context->Draw(3, 0);

    context->PSSetShaderResources(TEX_RT_SLOT_SRV, 1, &nullSRV);
    context->PSSetShaderResources(TEX_DEPTH_SLOT_SRV, 1, &nullSRV);
    context->PSSetShaderResources(TEX_CLOUD_SLOT_SRV, 1, &nullSRV);
} // ApplyGodRays

void PostEffects::ApplyLensflare(ID3D11DeviceContext* context,
    ID3D11ShaderResourceView* sceneSRV, ID3D11ShaderResourceView* cloudSRV, DirectX::XMFLOAT2 lightUV) {
    ID3D11ShaderResourceView* nullSRV = nullptr;

    ID3D11RenderTargetView* flareRTV = m_flareRT->GetRTV();
    context->OMSetRenderTargets(1, &flareRTV, nullptr);

    m_lensflareData.coreUV = lightUV;

    if (!UpdateConstantBuffer(context, m_lensflareBuffer.Get(), m_lensflareData)) {
        return;
    }

    context->PSSetConstantBuffers(CB_LENSFLARE_SLOT, 1, m_lensflareBuffer.GetAddressOf());
    context->PSSetShader(m_lensFlareShader.Get(), nullptr, 0);

    ID3D11ShaderResourceView* bloomSRV = m_bloomRT->GetSRV();
    context->PSSetShaderResources(TEX_RT_SLOT_SRV, 1, &bloomSRV);
    context->PSSetShaderResources(TEX_NOISE_SLOT_SRV, 1, &m_noiseSRV);
    context->PSSetShaderResources(TEX_COMPO_SLOT_SRV, 1, &sceneSRV);
    context->PSSetShaderResources(TEX_CLOUD_SLOT_SRV, 1, &cloudSRV);

    context->Draw(3, 0);

    context->PSSetShaderResources(TEX_RT_SLOT_SRV, 1, &nullSRV);
    context->PSSetShaderResources(TEX_DEPTH_SLOT_SRV, 1, &nullSRV);
    context->PSSetShaderResources(TEX_NOISE_SLOT_SRV, 1, &nullSRV);
} // ApplyLensflare

void PostEffects::ApplyComposite(ID3D11DeviceContext* context) {
    ID3D11ShaderResourceView* nullSRV = nullptr;

    ID3D11RenderTargetView* resRTV = m_resRT->GetRTV();
    context->OMSetRenderTargets(1, &resRTV, nullptr);
    context->PSSetShader(m_compositeShader.Get(), nullptr, 0);

    ID3D11ShaderResourceView* flareSRV = m_flareRT->GetSRV();
    ID3D11ShaderResourceView* raySRV = m_rayRT->GetSRV();

    context->PSSetShaderResources(TEX_C_FLARE_SLOT_SRV, 1, &flareSRV);
    context->PSSetShaderResources(TEX_C_RAY_SLOT_SRV, 1, &raySRV);

    context->Draw(3, 0);

    context->PSSetShaderResources(TEX_C_FLARE_SLOT_SRV, 1, &nullSRV);
    context->PSSetShaderResources(TEX_C_RAY_SLOT_SRV, 1, &nullSRV);
} // CouldPass

void PostEffects::OnGui() {
    if (ImGui::CollapsingHeader("POST EFFECTS", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();

        if (ImGui::TreeNodeEx("Lens Flare", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::ColorEdit3("Retouch Color", &m_lensflareData.retouchColor.x);

            ImGui::SliderFloat("Noise Scale", &m_lensflareData.lensScale, 0.0f, 0.05f, "%.4f");

            ImGui::SliderFloat("Core Tightness", &m_lensflareData.coreTight, 10.0f, 300.0f);
            ImGui::SliderFloat("Luminance Offset (LF)", &m_lensflareData.luminanceOffest, 0.0f, 1.0f);

            if (ImGui::Button("Reset Lens Flare")) {
                m_lensflareData = LensFlareBuffer();
            }
            ImGui::TreePop();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::TreeNodeEx("God Rays", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Density", &m_godRayData.density, 0.1f, 2.0f);

            ImGui::SliderFloat("Weight", &m_godRayData.weight, 0.01f, 0.2f, "%.3f");
            ImGui::SliderFloat("Decay", &m_godRayData.decay, 0.8f, 1.0f, "%.3f");
            ImGui::SliderFloat("Exposure", &m_godRayData.exposure, 0.1f, 3.0f);
            ImGui::SliderFloat("Luminance Threshold (GR)", &m_godRayData.luminanceThreshold, 0.0f, 1.0f);

            if (ImGui::Button("Reset God Rays")) {
                m_godRayData = GodRayBuffer();
            }
            ImGui::TreePop();
        }

        ImGui::Unindent();
    }
} // OnGui

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