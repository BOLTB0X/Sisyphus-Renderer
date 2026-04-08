#include "Pch.h"
#include "SkyBox.h"
// Resources
#include "Resources/DefaultMesh.h"
#include "Resources/VolumeTexture.h"
// Data
#include "Data/RenderTexture.h"
// D3D11
#include "D3D11/D3D11State.h"
// Post
#include "Post/Composite.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
// define
#define WEATHER_SIZE           1024
#define BUFFER_SLOT_WORLD      2
#define BUFFER_SLOT_ATMOSPHERE 3
#define BUFFER_SLOT_CLOUDBOX   4
#define SAMPLER_SLOT           0
#define TEX_SLOT_DEPTH         1
#define TEX_SLOT_WMAP          2
#define TEX_SLOT_BASE          3
#define TEX_SLOT_DETAIL        4

using namespace DirectX;
using namespace SharedConstants;
using namespace PathConstants;

SkyBox::SkyBox() {
	m_CubeMesh = std::make_unique<DefaultMesh>();
	m_Composite = std::make_unique<Composite>();
	m_volumetricRT = std::make_unique<RenderTexture>();
    m_linerWrapSampler = nullptr;
    m_depthSRV = nullptr;
	m_weatherSRV = nullptr;
	m_baseNoiseSRV = nullptr;
	m_detailNoiseSRV = nullptr;
    m_prevAtmosphereData.padding1.x = -1.0f;
    m_prevCloudBoxData.padding = -1.0f;
} // SkyBox

SkyBox::~SkyBox() {
    m_linerWrapSampler = nullptr;
    m_depthSRV = nullptr;
    m_weatherSRV = nullptr;
    m_baseNoiseSRV = nullptr;
    m_detailNoiseSRV = nullptr;
} // ~SkyBox

bool SkyBox::Init(const InitParams& params) {
    if (!params.device || !params.context || !params.baseNoise) {
        return false;
    }

    if (!m_CubeMesh->Init(params.device, 1, DefaultMesh::DefaultMeshType::Cube)) {
        return false;
    }

    if (!m_volumetricRT->Init(params.device,
        ScreenConstants::WIDTH / 2, ScreenConstants::HEIGHT / 2,
        RenderTexture::RenderTextureType::UAV, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        return false;
	}

    if (!InitShader(params.device, params.hwnd)) {
        return false;
    }

	Composite::InitParams compositeParams;
	compositeParams.device = params.device;
	compositeParams.hwnd = params.hwnd;
	compositeParams.vPath = PathConstants::COMPOSITE_VS;
	compositeParams.pPath = PathConstants::COMPOSITE_PS;
    if (!m_Composite->Init(compositeParams)) {
        return false;
	}

	m_weatherSRV = params.weather;
    m_baseNoiseSRV = params.baseNoise;
    m_detailNoiseSRV = params.detailNoise;
    m_linerWrapSampler = params.sampler;
    m_depthSRV = params.depth;
    return true;
} // Init

bool SkyBox::InitShader(ID3D11Device* device, HWND hwnd) {
    using namespace ShaderHelper;

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

    if (!InitConstantBuffer<WolrdBuffer>(device, m_worldBuffer.GetAddressOf()) ||
        !InitConstantBuffer<AtmosphereBuffer>(device, m_atmosphereBuffer.GetAddressOf()) ||
        !InitConstantBuffer<CloudBoxBuffer>(device, m_cloudBoxBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // InitShader

void SkyBox::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

    // [Step A] 저해상도 렌더링 준비
    // 현재 메인 렌더 타겟과 뷰포트를 저장, 나중에 복구용
    ID3D11RenderTargetView* mainRTV = nullptr;
    ID3D11DepthStencilView* mainDSV = nullptr;
    context->OMGetRenderTargets(1, &mainRTV, &mainDSV);

    D3D11_VIEWPORT oldViewport;
    UINT numViewports = 1;
    context->RSGetViewports(&numViewports, &oldViewport);

    // 저해상도 RT 청소 및 설정
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context->ClearRenderTargetView(m_volumetricRT->GetRTV(), clearColor);

    D3D11_VIEWPORT lowResViewport = { 0.0f, 0.0f, (float)SharedConstants::ScreenConstants::WIDTH / 2.0f, (float)SharedConstants::ScreenConstants::HEIGHT / 2.0f, 0.0f, 1.0f };
    context->RSSetViewports(1, &lowResViewport);

    ID3D11RenderTargetView* lowResRTV = m_volumetricRT->GetRTV();
    context->OMSetRenderTargets(1, &lowResRTV, nullptr); // 대기/구름은 깊이 쓰기 안 함

    // [Step B] 저해상도 RT에 SkyBox(대기+구름) 그리기
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    XMMATRIX world = XMMatrixScaling(500.0f, 500.0f, 500.0f);
    m_WolrdData.world = XMMatrixTranspose(world);
    ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_WolrdData);

    context->PSSetSamplers(SAMPLER_SLOT, 1, &m_linerWrapSampler);
    context->PSSetShaderResources(TEX_SLOT_DEPTH, 1, &m_depthSRV);
    context->PSSetShaderResources(TEX_SLOT_WMAP, 1, &m_weatherSRV);
    context->PSSetShaderResources(TEX_SLOT_BASE, 1, &m_baseNoiseSRV);
    context->PSSetShaderResources(TEX_SLOT_DETAIL, 1, &m_detailNoiseSRV);

    context->VSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_worldBuffer.GetAddressOf());
    context->PSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_worldBuffer.GetAddressOf());

    if (UpdateAtmosphereBuffer(context)) {
        context->PSSetConstantBuffers(BUFFER_SLOT_ATMOSPHERE, 1, m_atmosphereBuffer.GetAddressOf());
    }

    if (UpdateCloudBoxBuffer(context, params.camPos)) {
        context->PSSetConstantBuffers(BUFFER_SLOT_CLOUDBOX, 1, m_cloudBoxBuffer.GetAddressOf());
	}

    m_CubeMesh->RenderBuffer(context);
    context->DrawIndexed(m_CubeMesh->GetIndexCount(), 0, 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(TEX_SLOT_DEPTH, 1, &nullSRV);

    // [Step C] 메인 화면으로 복구 및 풀스크린 합성
    context->RSSetViewports(1, &oldViewport);
    context->OMSetRenderTargets(1, &mainRTV, mainDSV);

    // 합성용 셰이더 설정
    context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	m_Composite->Render(context, m_volumetricRT->GetSRV(), m_linerWrapSampler);
    // 리소스 반납
    ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr, nullptr, nullptr };
    context->PSSetShaderResources(0, 5, nullSRVs);

    if (mainRTV) mainRTV->Release();
    if (mainDSV) mainDSV->Release();
} // Render

void SkyBox::OnGui() {
    ImGui::Begin("Sky & Atmosphere Control");
    GuiAtmosphere();

    ImGui::Separator();
    
    ImGui::End();
} // OnGui

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

bool SkyBox::UpdateCloudBoxBuffer(ID3D11DeviceContext* context, const DirectX::XMFLOAT3& camPos) {
    using namespace ShaderHelper;

    float boxCenterY = m_cloudBoxData.cloudMinHeight + (m_cloudBoxData.boxSize.y * 0.5f); // 1500 + 1500 = 3000
    m_cloudBoxData.boxCenter = DirectX::XMFLOAT4(camPos.x, boxCenterY, camPos.z, 1.0f);

    if (memcmp(&m_prevCloudBoxData, &m_cloudBoxData, sizeof(CloudBoxBuffer)) == 0) {
        return true;
    }

    if (!UpdateConstantBuffer(context, m_cloudBoxBuffer.Get(), m_cloudBoxData)) {
        return false;
    }

    m_prevCloudBoxData = m_cloudBoxData;
    return true;
} // UpdateCloudBoxBuffer

void SkyBox::GuiAtmosphere() {
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
} // GuiAtmosphere
