#include "Pch.h"
#include "SkyBox.h"
#include "Resources/DefaultMesh.h"
#include "Resources/Texture.h"
#include "Resources/TextureManager.h"
// D3D11
#include "D3D11/D3D11State.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
#include "ImGui/ImGuiDrawer.h"
// define
#define BUFFER_SLOT_COMMON     0
#define BUFFER_SLOT_ATMOSPHERE 1
#define SAMPLER_SLOT           0

using namespace DirectX;
using namespace SharedConstants;
using namespace PathConstants;
using namespace ConstantBuffer;

SkyBox::SkyBox() {
	m_cubeMesh = std::make_unique<DefaultMesh>();
	m_sampler = nullptr;
	m_renderCount = 0;
    m_prevAtmosphereData.padding1.x = -1.0f;
    m_prevCommonData.padding1 = -1.0f;
} // SkyBox

SkyBox::~SkyBox() {
	m_sampler = nullptr;
} // ~SkyBox

bool SkyBox::Init(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd, ID3D11SamplerState* sampler, TextureManager* texMgr) {
	if (!m_cubeMesh->Init(device, 1, DefaultMesh::DefaultMeshType::Cube)) {
		return false;
	}
	if (!InitShader(device, hwnd)) { 
		return false;
	}

	m_sampler = sampler;
	return true;
} // Init

bool SkyBox::InitShader(ID3D11Device* device, HWND hwnd) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (!InitVertexShader(device, hwnd, PathConstants::SKYBOX_VS,
        layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(device, hwnd, PathConstants::SKYBOX_PS, m_pixelShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<CommonBuffer>(device, m_commonBuffer.GetAddressOf()) ||
        !InitConstantBuffer<AtmosphereBuffer>(device, m_atmosphereBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // InitShader

void SkyBox::Render(ID3D11DeviceContext* context, const RenderParams& params) {
	m_renderCount++;
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    XMMATRIX view = params.view;
    view.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMMATRIX world = XMMatrixScaling(500.0f, 500.0f, 500.0f);
    context->PSSetSamplers(SAMPLER_SLOT, 1, &m_sampler);

    if (UpdateCommonBuffer(context, world, view, params.projection, params.cameraPosition, params.lightDir, params.lightDiffuse)) {
        context->VSSetConstantBuffers(BUFFER_SLOT_COMMON, 1, m_commonBuffer.GetAddressOf());
        context->PSSetConstantBuffers(BUFFER_SLOT_COMMON, 1, m_commonBuffer.GetAddressOf());
    }
    if (UpdateAtmosphereBuffer(context))
        context->PSSetConstantBuffers(BUFFER_SLOT_ATMOSPHERE, 1, m_atmosphereBuffer.GetAddressOf());

    // 그리기
    m_cubeMesh->RenderBuffer(context);
    context->DrawIndexed(m_cubeMesh->GetIndexCount(), 0, 0);
} // Render

void SkyBox::OnGui() {
    ImGui::Begin("Sky & Atmosphere Control");
    GuiAtmosphereBuffer();
    ImGui::End();
} // OnGui


bool SkyBox::UpdateCommonBuffer(ID3D11DeviceContext* context,
    const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
    const XMFLOAT3& camPos, const XMFLOAT3& lightDir, const XMFLOAT4& lightDiff) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;

    CommonBuffer buffer;

    // 행렬 전치
    buffer.world = XMMatrixTranspose(world);
    buffer.view = XMMatrixTranspose(view);
    buffer.projection = XMMatrixTranspose(projection);
    buffer.viewInv = XMMatrixTranspose(XMMatrixInverse(nullptr, view));
    buffer.projInv = XMMatrixTranspose(XMMatrixInverse(nullptr, projection));
    buffer.cameraPosition = camPos;

    // LightDir 정규화 안전 처리
    XMVECTOR ld = XMLoadFloat3(&lightDir);
    if (XMVectorGetX(XMVector3Length(ld)) < 1e-6f) {
        buffer.lightDirection = { 0.0f, -1.0f, 0.0f };
    }
    else {
        XMStoreFloat3(&buffer.lightDirection, XMVector3Normalize(ld));
    }

    buffer.lightDiffuse = lightDiff;

    if (memcmp(&m_prevCommonData, &buffer, sizeof(CommonBuffer)) == 0) {
        return true;
    }

    if (!UpdateConstantBuffer(context, m_commonBuffer.Get(), buffer)) {
        return false;
    }

    m_prevCommonData = buffer;
    m_CommonData = buffer;
    return true;
} // UpdateCommonBuffer

bool SkyBox::UpdateAtmosphereBuffer(ID3D11DeviceContext* context) {
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

void SkyBox::GuiAtmosphereBuffer() {
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
        ImGui::ColorEdit3("Ground", &m_atmosphereData.groundColor.x);
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
} // GuiAtmosphereBuffer