#include "Pch.h"
#include "VolumetricFog.h"
#include "D3D11/D3D11State.h"
#include "RenderTexture.h"
// Utils
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
// define
#define SAMPLER_SLOT1           0
#define SAMPLER_SLOT2           1
#define RES_UAV_SLOT            0
#define TEX_SLOT_DEPTH          1
#define TEX_SLOT_NORMAL         2
#define TEX_SLOT_HEIGHTMAP      3
#define TEX_SLOT_WORLEY_NOISE   4
#define CONSTANS_SLOT1          2

using namespace SharedConstants;
using namespace ShaderHelper;
using namespace DirectX;
using namespace DebugHelper;

VolumetricFog::VolumetricFog() {
    m_resultRT = std::make_unique<RenderTexture>();
    m_linerWrapSampler = nullptr;
    m_pointClampSampler = nullptr;
    m_heightMapSRV = nullptr;
    m_worleyNoiseSRV = nullptr;
    m_prevFogBufferData.terrainWidth = -1.0f;
} // VolumetricFog

VolumetricFog::~VolumetricFog() {
    m_linerWrapSampler = nullptr;
    m_pointClampSampler = nullptr;
    m_heightMapSRV = nullptr;
    m_worleyNoiseSRV = nullptr;
} // ~VolumetricFog

bool VolumetricFog::Init(const InitParams& params) {
    if (!m_resultRT->Init(params.device, params.screenWidth, params.screenHeight,
        RenderTexture::RenderTextureType::UAV, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        DebugHelper::DebugPrint("Init");
        return false;
    }

    if (!InitComputingShader(params.device, params.hwnd,
        PathConstants::VOLUMETRIC_FOG_CS, m_computeShader.GetAddressOf())) {
        DebugHelper::DebugPrint("Failed to InitComputingShader");
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

    return true;
} // Init

void VolumetricFog::Execute(ID3D11DeviceContext* context, const ExecuteParams& params) {
    m_fogBufferData.terrainWidth = params.terrainWidth;
    m_fogBufferData.terrainDepth = params.terrainDepth;
    m_fogBufferData.terrainHeightScale = params.terrainHeightScale;

    if (UpdateVolumetricFogBuffer(context)) {
        context->CSSetConstantBuffers(CONSTANS_SLOT1, 1, m_fogBuffer.GetAddressOf());
    }

    ID3D11UnorderedAccessView* pUAV = m_resultRT->GetUAV();
    context->CSSetUnorderedAccessViews(RES_UAV_SLOT, 1, &pUAV, nullptr);

    context->CSSetSamplers(SAMPLER_SLOT1, 1, &m_linerWrapSampler);
    context->CSSetSamplers(SAMPLER_SLOT2, 1, &m_pointClampSampler);
    context->CSSetShaderResources(TEX_SLOT_DEPTH, 1, &params.depthSRV);
    context->CSSetShaderResources(TEX_SLOT_NORMAL, 1, &params.normalSRV);
    context->CSSetShaderResources(TEX_SLOT_HEIGHTMAP, 1, &m_heightMapSRV);
    context->CSSetShaderResources(TEX_SLOT_WORLEY_NOISE, 1, &m_worleyNoiseSRV);

    context->CSSetShader(m_computeShader.Get(), nullptr, 0);
    UINT gridX = static_cast<UINT>(std::ceil(m_resultRT->GetWidth() / 8.0f));
    UINT gridY = static_cast<UINT>(std::ceil(m_resultRT->GetHeight() / 8.0f));
    context->Dispatch(gridX, gridY, 1);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(RES_UAV_SLOT, 1, &nullUAV, nullptr);
} // Execute

void VolumetricFog::OnGui() {
    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Height Fog ]");
    ImGui::DragFloat("Base Height", &m_fogBufferData.fogBaseHeight, 0.5f, -100.0f, 500.0f, "%.1f");
    ImGui::DragFloat("Height Falloff", &m_fogBufferData.fogHeightFalloff, 0.001f, 0.0001f, 1.0f, "%.4f");
    ImGui::SliderFloat("Density", &m_fogBufferData.fogDensity, 0.0f, 1.0f, "%.3f");
    ImGui::DragFloat("Max Distance", &m_fogBufferData.fogMaxDistance, 10.0f, 100.0f, 20000.0f, "%.0f");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Patchiness (Noise) ]");
    ImGui::DragFloat("Noise Scale", &m_fogBufferData.fogNoiseScale, 0.0001f, 0.00001f, 0.01f, "%.5f");
    ImGui::SliderFloat("Noise Strength", &m_fogBufferData.fogNoiseStrength, 0.0f, 1.0f);
    ImGui::DragFloat("Wind Speed", &m_fogBufferData.fogWindSpeed, 0.01f, 0.0f, 10.0f, "%.2f");
    ImGui::DragFloat2("Wind Direction", &m_fogBufferData.fogWindDirection.x, 0.01f, -1.0f, 1.0f, "%.2f");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Slope Attenuation (Normal) ]");
    ImGui::SliderFloat("Slope Min", &m_fogBufferData.fogSlopeMin, 0.0f, 1.0f);
    ImGui::SliderFloat("Slope Max", &m_fogBufferData.fogSlopeMax, 0.0f, 1.0f);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Lighting ]");
    ImGui::ColorEdit3("Fog Color", &m_fogBufferData.fogColor.x, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
    ImGui::SliderFloat("Ambient Strength", &m_fogBufferData.fogAmbientStrength, 0.0f, 5.0f);
    ImGui::SliderFloat("Phase G (Scattering)", &m_fogBufferData.fogPhaseG, -0.99f, 0.99f);
    ImGui::SliderInt("March Steps", &m_fogBufferData.fogMarchSteps, 8, 64);
    ImGui::Separator();

    if (m_resultRT) {
        ImGui::Text("VolumetricFog Preview");
        ImGui::Image((ImTextureID)m_resultRT->GetSRV(), ImVec2(256, 256));
    }
} // OnGui

ID3D11ShaderResourceView* VolumetricFog::GetFogSRV() {
    return m_resultRT->GetSRV();
} // GetFogSRV

bool VolumetricFog::UpdateVolumetricFogBuffer(ID3D11DeviceContext* context) {
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