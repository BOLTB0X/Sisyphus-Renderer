#include "Pch.h"
#include "FogComposite.h"
// Data
#include "Data/RenderTexture.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "Helpers/ShaderHelper.h"
#include "Helpers/DebugHelper.h"
// define
#define SAMPLER_SLOT1           0
#define SAMPLER_SLOT2           1
#define TEX_SLOT_SCENE          0
#define TEX_SLOT_DEPTH          1
#define TEX_SLOT_NOISE          2
#define TEX_SLOT_HEIGHTMAP      3
#define TEX_SLOT_WORLEY_NOISE   4
#define TEX_SLOT_NOMARL         5
#define CONSTANS_SLOT1          2

using namespace SharedConstants;
using namespace ShaderHelper;
using namespace DebugHelper;

FogComposite::FogComposite() {
    m_compositeRT = std::make_unique<RenderTexture>();
    m_linerWrapSampler = nullptr;
    m_pointClampSampler = nullptr;
    m_heightMapSRV = nullptr;
    m_worleyNoiseSRV = nullptr;
    m_noiseMapSRV = nullptr;
    m_prevFogBufferData.terrainWidth = -1.0f;
} // FogComposite

FogComposite::~FogComposite() {
    m_linerWrapSampler = nullptr;
    m_pointClampSampler = nullptr;
    m_heightMapSRV = nullptr;
    m_worleyNoiseSRV = nullptr;
    m_noiseMapSRV = nullptr;
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


    if (!InitConstantBuffer<VolumetricFogBuffer>(params.device, m_fogBuffer.GetAddressOf())) {
        DebugHelper::DebugPrint("Failed to create constant buffer for VolumetricFog.");
        return false;
    }

    m_linerWrapSampler = params.wrapSampler;
    m_pointClampSampler = params.pointSampler;
    m_heightMapSRV = params.heightMapSRV;
    m_worleyNoiseSRV = params.worleyNoiseSRV;
    m_noiseMapSRV = params.noiseMapSRV;
    return true;
} // Init

void FogComposite::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    m_fogBufferData.terrainWidth = params.terrainWidth;
    m_fogBufferData.terrainDepth = params.terrainDepth;
    m_fogBufferData.terrainHeightScale = params.terrainHeightScale;

    if (UpdateVolumetricFogBuffer(context)) {
        context->PSSetConstantBuffers(CONSTANS_SLOT1, 1, m_fogBuffer.GetAddressOf());
    }


    context->PSSetSamplers(SAMPLER_SLOT1, 1, &m_linerWrapSampler);
    context->PSSetSamplers(SAMPLER_SLOT2, 1, &m_pointClampSampler);
    context->PSSetShaderResources(TEX_SLOT_SCENE, 1, &params.sceneSRV);
    context->PSSetShaderResources(TEX_SLOT_DEPTH, 1, &params.depthSRV);
    context->PSSetShaderResources(TEX_SLOT_NOISE, 1, &m_noiseMapSRV);
    context->PSSetShaderResources(TEX_SLOT_HEIGHTMAP, 1, &m_heightMapSRV);
    context->PSSetShaderResources(TEX_SLOT_WORLEY_NOISE, 1, &m_worleyNoiseSRV);
    context->PSSetShaderResources(TEX_SLOT_NOMARL, 1, &params.normalSRV);


    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    context->Draw(3, 0);


    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
} // Render

void FogComposite::OnGui() {
    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Height Fog ]");
    ImGui::DragFloat("Base Height", &m_fogBufferData.fogBaseHeight, 0.5f, -100.0f, 500.0f, "%.1f");
    ImGui::DragFloat("Height Falloff", &m_fogBufferData.fogHeightFalloff, 0.001f, 0.0001f, 1.0f, "%.4f");
    ImGui::SliderFloat("Density", &m_fogBufferData.fogDensity, 0.0f, 1.0f, "%.005f");
    ImGui::DragFloat("Max Distance", &m_fogBufferData.fogMaxDistance, 10.0f, 100.0f, 20000.0f, "%.0f");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Patchiness (Noise) ]");
    ImGui::DragFloat("Noise Scale", &m_fogBufferData.fogNoiseScale, 0.0001f, 0.00001f, 0.01f, "%.5f");
    ImGui::SliderFloat("Noise Strength", &m_fogBufferData.fogNoiseStrength, 0.0f, 1.0f);
    ImGui::DragFloat("Wind Speed", &m_fogBufferData.fogWindSpeed, 0.01f, 0.0f, 10.0f, "%.2f");
    ImGui::DragFloat2("Wind Direction", &m_fogBufferData.fogWindDirection.x, 0.01f, -1.0f, 1.0f, "%.2f");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Lighting ]");
    ImGui::ColorEdit3("Fog Color", &m_fogBufferData.fogColor.x, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
    ImGui::SliderFloat("Ambient Strength", &m_fogBufferData.fogAmbientStrength, 0.0f, 5.0f);
    ImGui::SliderInt("March Steps", &m_fogBufferData.fogMarchSteps, 8, 64);
    ImGui::Separator();
} // OnGui

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

bool FogComposite::UpdateVolumetricFogBuffer(ID3D11DeviceContext* context) {
    using namespace ShaderHelper;

    if (memcmp(&m_prevFogBufferData, &m_fogBufferData, sizeof(VolumetricFogBuffer)) == 0) {
        return true;
    }

    if (!UpdateConstantBuffer(context, m_fogBuffer.Get(), m_fogBufferData)) {
        return false;
    }

    m_prevFogBufferData = m_fogBufferData;
    return true;
} // UpdateVolumetricFogBuffer