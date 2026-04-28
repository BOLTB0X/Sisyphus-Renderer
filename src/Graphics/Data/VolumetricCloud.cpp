#include "Pch.h"
#include "VolumetricCloud.h"
#include "D3D11/D3D11State.h"
#include "RenderTexture.h"
//Utils
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"
// define
#define SAMPLER_SLOT1          0
#define SAMPLER_SLOT2          1
#define UAV_SLOT               0
#define TEX_SLOT_DEPTH         1
#define TEX_SLOT_CLOUD_LUT     2
#define TEX_SLOT_WORLEY_NOISE  3
#define TEX_SLOT_BLUE_NOISE    4
#define TEX_SLOT_SKY_LUT       5
#define CONSTANS_SLOT1         2

using namespace SharedConstants;
using namespace ShaderHelper;
using namespace DirectX;
using namespace DebugHelper;

VolumetricCloud::VolumetricCloud() {
	m_resultRT = std::make_unique<RenderTexture>();
	m_linerWrapSampler = nullptr;
	m_pointClampSampler = nullptr;
	m_cloudMapLUTSRV = nullptr;
	m_worleyNoiseSRV = nullptr;
	m_blueNoiseSRV = nullptr;
	m_prevCloudBufferData.padding1 = -1.0f;
} // VolumetricCloud

VolumetricCloud::~VolumetricCloud() {
	m_linerWrapSampler = nullptr;
	m_pointClampSampler = nullptr;
	m_cloudMapLUTSRV = nullptr;
	m_worleyNoiseSRV = nullptr;
	m_blueNoiseSRV = nullptr;
} // ~VolumetricCloud

bool VolumetricCloud::Init(const InitParams& params) {
	UINT halfWidth = SharedConstants::ScreenConstants::WIDTH / 2;
	UINT halfHeight = SharedConstants::ScreenConstants::HEIGHT / 2;

    if (!m_resultRT->Init(params.device, halfWidth, halfHeight,
        RenderTexture::RenderTextureType::UAV, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        return false;
    }

    if (!InitComputingShader(params.device, params.hwnd, 
		PathConstants::VOLUMETRIC_CLOUD_CS, m_computeShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<VolumetricCloudBuffer>(params.device, m_cloudBuffer.GetAddressOf())) {
        return false;
    }

	m_linerWrapSampler = params.wrapSampler;
	m_pointClampSampler = params.pointSampler;
	m_cloudMapLUTSRV = params.cloudMapLUTSRV;
	m_worleyNoiseSRV = params.worleyNoiseSRV;
	m_blueNoiseSRV = params.blueNoiseSRV;

    return true;
} // Init

void VolumetricCloud::Execute(ID3D11DeviceContext* context, const ExecuteParams& params) {
	if (UpdateVolumetricCloudBuffer(context)) {
		context->CSSetConstantBuffers(CONSTANS_SLOT1, 1, m_cloudBuffer.GetAddressOf());
	}

	ID3D11UnorderedAccessView* pUAV = m_resultRT->GetUAV();
	context->CSSetUnorderedAccessViews(UAV_SLOT, 1, &pUAV, nullptr);

	context->CSSetSamplers(SAMPLER_SLOT1, 1, &m_linerWrapSampler);
	context->CSSetSamplers(SAMPLER_SLOT2, 1, &m_pointClampSampler);
	context->CSSetShaderResources(TEX_SLOT_DEPTH, 1, &params.depthSRV);
	context->CSSetShaderResources(TEX_SLOT_CLOUD_LUT, 1, &m_cloudMapLUTSRV);
	context->CSSetShaderResources(TEX_SLOT_WORLEY_NOISE, 1, &m_worleyNoiseSRV);
	context->CSSetShaderResources(TEX_SLOT_BLUE_NOISE, 1, &m_blueNoiseSRV);
	context->CSSetShaderResources(TEX_SLOT_SKY_LUT, 1, &params.SkyLUTSRV);

	context->CSSetShader(m_computeShader.Get(), nullptr, 0);
	UINT gridX = static_cast<UINT>(std::ceil(m_resultRT->GetWidth() / 8.0f));
	UINT gridY = static_cast<UINT>(std::ceil(m_resultRT->GetHeight() / 8.0f));
	context->Dispatch(gridX, gridY, 1);

	ID3D11UnorderedAccessView* nullUAV = nullptr;
	context->CSSetUnorderedAccessViews(UAV_SLOT, 1, &nullUAV, nullptr);
} // Execute

void VolumetricCloud::OnGui() {
    // Reset 버튼
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

    if (ImGui::Button("Reset to Default", ImVec2(-1, 0))) {
        m_cloudBufferData = VolumetricCloudBuffer();
    }

    ImGui::PopStyleColor(3);
    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.3f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.4f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.3f, 0.2f, 1.0f));

    if (ImGui::CollapsingHeader("CLOUD SETTINGS", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PopStyleColor(3);
        ImGui::Indent();
        ImGui::Spacing();

        // [ Planet ]
        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Planet ]");
        ImGui::DragFloat("Planet Radius", &m_cloudBufferData.planetRadius,
            1000.0f, 1000.0f, 1e7f, "%.0f");
        ImGui::Separator();

        // [ Cloud Layer ]
        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Cloud Layer ]");
        ImGui::DragFloat("Cloud Bottom", &m_cloudBufferData.cloudBottom,
            10.0f, 0.0f, 10000.0f, "%.0f");
        ImGui::DragFloat("Cloud Top", &m_cloudBufferData.cloudTop,
            10.0f, 0.0f, 10000.0f, "%.0f");
        ImGui::DragFloat("Layer Bottom", &m_cloudBufferData.cloudsLayerBottom,
            1.0f, -1000.0f, 0.0f, "%.0f");
        ImGui::DragFloat("Layer Top", &m_cloudBufferData.cloudsLayerTop,
            1.0f, -1000.0f, 0.0f, "%.0f");
        ImGui::Separator();

        // [ Shape ]
        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Shape ]");
        ImGui::SliderFloat("Coverage", &m_cloudBufferData.cloudCoverage,
            0.0f, 1.0f);
        ImGui::SliderFloat("Layer Coverage", &m_cloudBufferData.cloudsLayerCoverage,
            0.0f, 1.0f);
        ImGui::DragFloat("Base Scale", &m_cloudBufferData.cloudBaseScale,
            0.01f, 0.01f, 10.0f, "%.2f");
        ImGui::DragFloat("Detail Scale", &m_cloudBufferData.cloudDetailScale,
            0.1f, 0.1f, 100.0f, "%.1f");
        ImGui::Separator();

        // [ Density ]
        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Density ]");
        ImGui::DragFloat("Density", &m_cloudBufferData.cloudDensity,
            0.001f, 0.0f, 1.0f, "%.4f");
        ImGui::SliderFloat("Edge Softness", &m_cloudBufferData.baseEdgeSoftness,
            0.0f, 1.0f);
        ImGui::SliderFloat("Bottom Softness", &m_cloudBufferData.bottomSoftness,
            0.0f, 1.0f);
        ImGui::SliderFloat("Detail Strength", &m_cloudBufferData.detailStrength,
            0.0f, 1.0f);
        ImGui::Separator();

        // [ Lighting ]
        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Lighting ]");
        ImGui::SliderFloat("Forward Scatter G", &m_cloudBufferData.forwardScatteringG,
            -0.99f, 0.99f);
        ImGui::SliderFloat("Backward Scatter G", &m_cloudBufferData.backwardScatteringG,
            -0.99f, 0.99f);
        ImGui::SliderFloat("Scattering Lerp", &m_cloudBufferData.scatteringLerp,
            0.0f, 1.0f);
        ImGui::SliderFloat("Min Transmittance", &m_cloudBufferData.minTransmittance,
            0.0f, 1.0f);
        ImGui::SliderFloat("HG Scale", &m_cloudBufferData.hgScale,
            0.0f, 5.0f);
        ImGui::SliderFloat("Powder Factor", &m_cloudBufferData.PowderFactor,
            0.0f, 1.0f);
        ImGui::SliderFloat("Lighting Scale", &m_cloudBufferData.LightingScale,
            0.0f, 5.0f);
        ImGui::SliderFloat("Horizon Fade", &m_cloudBufferData.HorizenFadeScale,
            0.0f, 1.0f);
        ImGui::Separator();

        // [ Ambient ]
        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Day Ambient ]");
        ImGui::ColorEdit3("Ambient Top", &m_cloudBufferData.ambientTop.x);
        ImGui::ColorEdit3("Ambient Bottom", &m_cloudBufferData.ambientBottom.x);
        ImGui::Separator();

        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ SunSet Ambient ]");
        ImGui::ColorEdit3("Ambient Top", &m_cloudBufferData.sunsetAmbientTop.x);
        ImGui::ColorEdit3("Ambient Bottom", &m_cloudBufferData.sunsetAmbientBottom.x);
        ImGui::Separator();

        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Night Ambient ]");
        ImGui::ColorEdit3("Ambient Top", &m_cloudBufferData.nightAmbientTop.x);
        ImGui::ColorEdit3("Ambient Bottom", &m_cloudBufferData.nightAmbientBottom.x);
        ImGui::Separator();

        // [ Wind ]
        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "[ Wind ]");
        ImGui::DragFloat2("Wind Direction", &m_cloudBufferData.windDirection.x,
            0.01f, -1.0f, 1.0f, "%.2f");
        ImGui::DragFloat("Wind Speed", &m_cloudBufferData.windSpeed,
            0.01f, 0.0f, 10.0f, "%.2f");
        ImGui::DragFloat("Wind Scale", &m_cloudBufferData.windScale,
            0.01f, 0.0f, 10.0f, "%.2f");

        ImGui::Unindent();
    }
    else {
        ImGui::PopStyleColor(3);
    }
    ImGui::Separator();

	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[ Debug Textures ]");

	ImGui::Spacing();

	if (m_resultRT) {
		ImGui::Text("VolumetricCloud Preview");
		ImGui::Image((ImTextureID)m_resultRT->GetSRV(), ImVec2(256, 256));
	}

	if (m_cloudMapLUTSRV) {
		ImGui::Text("CloudLUT");
		ImGui::Image((ImTextureID)m_cloudMapLUTSRV, ImVec2(256, 256));
	}

	ImGui::Spacing();

	if (m_worleyNoiseSRV) {
		ImGui::Text("Worley Noise");
		ImGui::Image((ImTextureID)m_worleyNoiseSRV, ImVec2(256, 256));
	}

	ImGui::Spacing();

	if (m_blueNoiseSRV) {
		ImGui::Text("Blue Noise");
		ImGui::Image((ImTextureID)m_blueNoiseSRV, ImVec2(256, 256));
	}
} // OnGui

ID3D11ShaderResourceView* VolumetricCloud::GetCloudSRV() {
	return m_resultRT->GetSRV();
} // GetCloudSRV

bool VolumetricCloud::UpdateVolumetricCloudBuffer(ID3D11DeviceContext* context) {
	using namespace ShaderHelper;

	if (memcmp(&m_prevCloudBufferData, &m_cloudBufferData, sizeof(VolumetricCloudBuffer)) == 0) {
		return true;
	}

	if (!UpdateConstantBuffer(context, m_cloudBuffer.Get(), m_cloudBufferData)) {
		return false;
	}

	m_prevCloudBufferData = m_cloudBufferData;
    return true;
} // UpdateVolumetricCloudBuffer
