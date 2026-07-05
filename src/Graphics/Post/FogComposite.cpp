#include "Pch.h"
#include "FogComposite.h"
// Data
#include "Data/RenderTexture.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "Helpers/ShaderHelper.h"
#include "Helpers/DebugHelper.h"
// define
#define SAMPLER_SLOT      0
#define TEX_SCENE_SLOT    0
#define TEX_FOG_SLOT      1

using namespace SharedConstants;
using namespace ShaderHelper;
using namespace DebugHelper;

FogComposite::FogComposite() {
    m_compositeRT = std::make_unique<RenderTexture>();
} // FogComposite

FogComposite::~FogComposite() {
} // ~FogComposite

bool FogComposite::Init(const InitParams& params) {
    if (!params.device || !params.hwnd) {
        return false;
    }

    if (!m_compositeRT->Init(params.device, params.screenWidth, params.screenHeight,
        RenderTexture::RenderTextureType::Normal, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        DebugHelper::DebugPrint("FogComposite m_compositeRT 초기화 실패");
        return false;
    }

    if (!InitShader(params.device, params.hwnd)) {
        DebugHelper::DebugPrint("FogComposite InitShader 초기화 실패");
        return false;
    }

    return true;
} // Init

void FogComposite::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    context->PSSetShaderResources(TEX_SCENE_SLOT, 1, &params.sceneSRV);
    context->PSSetShaderResources(TEX_FOG_SLOT, 1, &params.fogSRV);

    context->Draw(3, 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(TEX_SCENE_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_FOG_SLOT, 1, &nullSRV);
} // Render

void FogComposite::ClearRT(ID3D11DeviceContext* context) {
    m_compositeRT->Clear(context);
} // ClearRT

ID3D11Texture2D* FogComposite::GetTexture() const {
    return m_compositeRT->GetTexture();
} // GetTexture

ID3D11RenderTargetView* FogComposite::GetRTV() const {
    return m_compositeRT->GetRTV();
} // GetRTV

ID3D11ShaderResourceView* FogComposite::GetSRV() const {
    return m_compositeRT->GetSRV();
} // GetSRV

bool FogComposite::InitShader(ID3D11Device* device, HWND hwnd) {
    if (InitVertexShader(device, hwnd, PathConstants::POST_VS,
        nullptr, 0, m_vertexShader.GetAddressOf(), nullptr) == false) {
        DebugHelper::DebugPrint("POST_VS 초기화 실패");
        return false;
    }

    if (InitPixelShader(device, hwnd, PathConstants::FOG_COMPOSITE_PS,
        m_pixelShader.GetAddressOf()) == false) {
        DebugHelper::DebugPrint("FogComposite PS 초기화 실패");
        return false;
    }

    return true;
} // InitShader