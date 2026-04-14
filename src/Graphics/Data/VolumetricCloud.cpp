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
#define SAMPLER_SLOT           0
#define UAV_SLOT               0
#define TEX_SLOT_CLOUD_LUT     1
#define TEX_SLOT_WORLEY_NOISE  2
#define TEX_SLOT_BLUE_NOISE    3
#define TEX_SLOT_SKY_LUT       4
#define CONSTANS_SLOT1         3
#define CONSTANS_SLOT2         4

using namespace SharedConstants;
using namespace ShaderHelper;
using namespace DirectX;
using namespace DebugHelper;

VolumetricCloud::VolumetricCloud() {
	m_resultRT = std::make_unique<RenderTexture>();
	m_linerWrapSampler = nullptr;
	m_cloudMapLUTSRV = nullptr;
	m_worleyNoiseSRV = nullptr;
	m_blueNoiseSRV = nullptr;
	m_prevCloudBufferData.padding1 = -1.0f;
} // VolumetricCloud

VolumetricCloud::~VolumetricCloud() {
	m_linerWrapSampler = nullptr;
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

	m_linerWrapSampler = params.sampler;
	m_cloudMapLUTSRV = params.CloudMapLUTSRV;
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

	context->CSSetSamplers(SAMPLER_SLOT, 1, &m_linerWrapSampler);
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
