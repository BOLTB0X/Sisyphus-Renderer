#include "Pch.h"
#include "Atmosphere.h"
#include "RenderTexture.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

// define
#define SAMPLER_SLOT           0
#define UAV_SLOT_LUT           0
#define TEX_SLOT_DEPTH         1
#define TEX_SLOT_LUT           2
#define CONSTANS_SLOT1         3
#define CONSTANS_SLOT2         4

using namespace DirectX;
using namespace SharedConstants;
using namespace PathConstants;
using namespace ConstantBuffer;
using namespace ShaderHelper;

Atmosphere::Atmosphere() {
	m_LUT = std::make_unique<RenderTexture>();
    m_prevAtmosphereData.padding1.x = -1.0f;
    m_linerWrapSampler = nullptr;
	m_prevResolutionData.padding.x = -1.0f;
	m_preLightDirection = { -1.0f, -1.0f, 999999.0f };
	m_preCameraPosition = { -999999.0f, -999999.0f, -999999.0f };
} // Atmosphere

Atmosphere::~Atmosphere() {
    m_linerWrapSampler = nullptr;
} // ~Atmosphere

bool Atmosphere::Init(const InitParams& params) {
    if (!params.device || !params.context) {
        return false;
    }

	m_linerWrapSampler = params.linerWrapSampler;

    if (!m_LUT->Init(params.device, m_resolutionData.resolution.x, m_resolutionData.resolution.y, 
        RenderTexture::RenderTextureType::UAV, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        return false;
	}

    if (!InitComputingShader(params.device, params.hwnd, ATMOSPHERE_CS, m_computeShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<AtmosphereBuffer>(params.device, m_atmosphereBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<ResolutionBuffer>(params.device, m_resolutionBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // Init

void Atmosphere::Execute(ID3D11DeviceContext* context, const ExecuteParams& param) {
    if (UpdateAtmosphereBuffer(context)) {
        context->CSSetConstantBuffers(CONSTANS_SLOT1, 1, m_atmosphereBuffer.GetAddressOf());
    }

    if (UpdateResolutionBuffer(context)) {
        context->CSSetConstantBuffers(CONSTANS_SLOT2, 1, m_resolutionBuffer.GetAddressOf());
	}

    if (param.LightDirection.x != m_preLightDirection.x ||
        param.LightDirection.y != m_preLightDirection.y ||
        param.LightDirection.z != m_preLightDirection.z ||
        param.CameraPosition.y != m_preCameraPosition.y) {
		m_preLightDirection = param.LightDirection;
        m_preCameraPosition = param.CameraPosition;
        Compute(context);
	}
} // Execute

ID3D11ShaderResourceView* Atmosphere::GetLUT() {
	return m_LUT->GetSRV();
} // GetLUT

void Atmosphere::OnGui() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

    if (ImGui::Button("Reset to Default", ImVec2(-1, 0))) {
        m_atmosphereData = AtmosphereBuffer();
    }
    ImGui::PopStyleColor(3);
    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.3f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.4f, 0.5f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.3f, 0.4f, 1.0f));

    if (ImGui::CollapsingHeader("ATMOSPHERE SETTINGS", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PopStyleColor(3);
        ImGui::Indent();
        ImGui::Spacing();

        // [ Colors ]
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "[ Colors ]");
        ImGui::ColorEdit4("Zenith", &m_atmosphereData.zenithColor.x);
        ImGui::ColorEdit4("Horizon", &m_atmosphereData.horizonColor.x);
        ImGui::Separator();

        // [ Geometry ]
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "[ Geometry ]");
        ImGui::DragFloat("Planet Radius", &m_atmosphereData.planetRadius, 1000.0f, 1000.0f, 1e7f, "%.0f");
        ImGui::DragFloat("Atmo Radius", &m_atmosphereData.atmoRadius, 1000.0f, 1000.0f, 1e7f, "%.0f");
        ImGui::DragFloat3("Planet Center", &m_atmosphereData.planetCenter.x, 100.0f, -1e7f, 1e7f, "%.0f");
        ImGui::Separator();

        // [ Scattering Coefficients ]
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "[ Scattering Coefficients ]");
        ImGui::DragFloat3("Rayleigh Beta", &m_atmosphereData.rayleighBeta.x, 0.000001f, 0.0f, 0.1f, "%.6f");
        ImGui::DragFloat("Mie Beta", &m_atmosphereData.mieBeta, 0.000001f, 0.0f, 0.1f, "%.6f");
        ImGui::DragFloat3("Absorption Beta", &m_atmosphereData.absorptionBeta.x, 0.000001f, 0.0f, 0.1f, "%.6f");
        ImGui::DragFloat("Ambient Beta", &m_atmosphereData.ambientBeta, 0.00001f, 0.0f, 0.1f, "%.5f");
        ImGui::Separator();

        // [ Heights & Phase ]
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "[ Heights & Phase ]");
        ImGui::DragFloat("Rayleigh Height", &m_atmosphereData.rayleighHeight, 100.0f, 100.0f, 50000.0f, "%.0f");
        ImGui::DragFloat("Mie Height", &m_atmosphereData.mieHeight, 100.0f, 100.0f, 50000.0f, "%.0f");
        ImGui::DragFloat("Absorb Height", &m_atmosphereData.absorptionHeight, 100.0f, 100.0f, 50000.0f, "%.0f");
        ImGui::SliderFloat("Mie G (Phase)", &m_atmosphereData.g, -0.99f, 0.99f);
        ImGui::DragFloat("Intensity", &m_atmosphereData.intensity, 0.1f, 0.0f, 100.0f, "%.2f");
        ImGui::Separator();

        // [ Raymarching Steps ]
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "[ Raymarching Steps ]");
        ImGui::SliderInt("Primary Steps", &m_atmosphereData.primarySteps, 4, 128);
        ImGui::SliderInt("Light Steps", &m_atmosphereData.lightSteps, 1, 32);

        ImGui::Unindent();
    }
    else {
        ImGui::PopStyleColor(3);
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[ Debug Textures ]");

    ImGui::Spacing();

    static int sliceIndex = 0;

    if (m_LUT) {
        ImGui::Text("Volume SRV Preview (Base Layer):");
        ImGui::Image((ImTextureID)m_LUT->GetSRV(), ImVec2(256, 256));
    }

} // OnGui

void Atmosphere::Compute(ID3D11DeviceContext* context) {
    ID3D11UnorderedAccessView* pUAV = m_LUT->GetUAV();

    context->CSSetShader(m_computeShader.Get(), nullptr, 0);
    context->CSSetSamplers(SAMPLER_SLOT, 1, &m_linerWrapSampler);
    context->CSSetUnorderedAccessViews(UAV_SLOT_LUT, 1, &pUAV, nullptr);
    UINT gridX = static_cast<UINT>(std::ceil(m_LUT->GetWidth() / 8.0f));
    UINT gridY = static_cast<UINT>(std::ceil(m_LUT->GetHeight() / 8.0f));
    context->Dispatch(gridX, gridY, 1);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(UAV_SLOT_LUT, 1, &nullUAV, nullptr);
} // Compute

bool Atmosphere::UpdateAtmosphereBuffer(ID3D11DeviceContext* context) {
    using namespace ShaderHelper;

    if (memcmp(&m_prevAtmosphereData, &m_atmosphereData, sizeof(AtmosphereBuffer)) == 0) {
        return true;
    }
    if (!UpdateConstantBuffer(context, m_atmosphereBuffer.Get(), m_atmosphereData)) {
        return false;
    }

    m_prevAtmosphereData = m_atmosphereData;
    return true;
} // UpdateAtmosphereBuffer

bool Atmosphere::UpdateResolutionBuffer(ID3D11DeviceContext* context) {
    using namespace ShaderHelper;
    
    if (memcmp(&m_prevResolutionData, &m_resolutionData, sizeof(ResolutionBuffer)) == 0) {
        return true;
    }

    if (!UpdateConstantBuffer(context, m_resolutionBuffer.Get(), m_resolutionData)) {
        return false;
    }

	m_prevResolutionData = m_resolutionData;
	return true;
} // UpdateResolutionBuffer