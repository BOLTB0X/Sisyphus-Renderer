#include "Pch.h"
#include "SkyBox.h"
#include "Resources/DefaultMesh.h"
#include "Resources/CloudMap.h"
#include "Resources/VolumeTexture.h"
// D3D11
#include "D3D11/RenderTexture.h"
#include "D3D11/D3D11State.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
// define
#define WEATHER_SIZE           512
#define BUFFER_SLOT_COMMON     0
#define BUFFER_SLOT_ATMOSPHERE 1
#define BUFFER_SLOT_CLOUD      2
#define SAMPLER_SLOT           0
#define TEX_SLOT_DEPTH         1
#define TEX_SLOT_VOLUME        2
#define TEX_SLOT_WMAP          3

using namespace DirectX;
using namespace SharedConstants;
using namespace PathConstants;
using namespace ConstantBuffer;

SkyBox::SkyBox() {
	m_CubeMesh = std::make_unique<DefaultMesh>();
	m_CloudMap = std::make_unique<CloudMap>();
    m_weatherMapRT = std::make_unique<RenderTexture>();
    m_linerWrapSampler = nullptr;
    m_depthSRV = nullptr;
    m_prevAtmosphereData.padding1.x = -1.0f;
    m_prevCloudData.padding.x = -1.0f;
    m_prevCommonData.padding1 = -1.0f;
} // SkyBox

SkyBox::~SkyBox() {
    m_linerWrapSampler = nullptr;
    m_depthSRV = nullptr;
} // ~SkyBox

bool SkyBox::Init(const InitParams& params) {
    if (!params.device || !params.context || !params.noiseTexture) {
        return false;
    }

    if (!m_CubeMesh->Init(params.device, 1, DefaultMesh::DefaultMeshType::Cube)) {
        return false;
    }

    if (!m_weatherMapRT->Init(params.device, WEATHER_SIZE, WEATHER_SIZE,
        RenderTexture::RenderTextureType::UAV, DXGI_FORMAT_R16G16_FLOAT)) {
        return false;
    }

    if (!m_CloudMap->Init(params.device, params.hwnd, PathConstants::CLOUDMAP_CS)) {
        return false;
    }

    if (!InitShader(params.device, params.hwnd)) {
        return false;
    }

    m_noise = params.noiseTexture;
    m_linerWrapSampler = params.sampler;
    m_depthSRV = params.depth;
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
        !InitConstantBuffer<AtmosphereBuffer>(device, m_atmosphereBuffer.GetAddressOf()) ||
        !InitConstantBuffer<CloudBuffer>(device, m_cloudBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // InitShader

void SkyBox::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    static bool isFirst = false;
    if (!isFirst) {
        m_cloudMapData.time = params.time * 0.05;
        m_CloudMap->Generate(context, m_weatherMapRT->GetUAV(), m_cloudMapData);
        isFirst = true;
    }
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    XMMATRIX view = params.view;
    view.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMMATRIX world = XMMatrixScaling(500.0f, 500.0f, 500.0f);
    auto noiseSRV = m_noise->GetSRV();
    auto weatherSRV = m_weatherMapRT->GetSRV();

    context->PSSetSamplers(SAMPLER_SLOT, 1, &m_linerWrapSampler);
    context->PSSetShaderResources(TEX_SLOT_DEPTH, 1, &m_depthSRV);
    context->PSSetShaderResources(TEX_SLOT_VOLUME, 1, &noiseSRV);
    context->PSSetShaderResources(TEX_SLOT_WMAP, 1, &weatherSRV);

    if (UpdateCommonBuffer(context, world, view, params.projection, params.cameraPosition, params.lightDir, params.lightDiffuse)) {
        context->VSSetConstantBuffers(BUFFER_SLOT_COMMON, 1, m_commonBuffer.GetAddressOf());
        context->PSSetConstantBuffers(BUFFER_SLOT_COMMON, 1, m_commonBuffer.GetAddressOf());
    }

    if (UpdateAtmosphereBuffer(context)) {
        context->PSSetConstantBuffers(BUFFER_SLOT_ATMOSPHERE, 1, m_atmosphereBuffer.GetAddressOf());
    }

    if (UpdateCloudBuffer(context)) {
        context->PSSetConstantBuffers(BUFFER_SLOT_CLOUD, 1, m_cloudBuffer.GetAddressOf());
    }

    // 그리기
    m_CubeMesh->RenderBuffer(context);
    context->DrawIndexed(m_CubeMesh->GetIndexCount(), 0, 0);
} // Render

void SkyBox::OnGui() {
    ImGui::Begin("Sky & Atmosphere Control");
    GuiAtmosphere();

    ImGui::Separator();

    GuiWeatherMap();
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

bool SkyBox::UpdateCloudBuffer(ID3D11DeviceContext* context) {
    using namespace ShaderHelper;

    if (memcmp(&m_prevCloudData, &m_cloudData, sizeof(CloudBuffer)) == 0) {
        return true;
    }

    if (!UpdateConstantBuffer(context, m_cloudBuffer.Get(), m_cloudData)) {
        return false;
    }

    m_prevCloudData = m_cloudData;
    return true;
} // UpdateCloudBuffer

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
        //ImGui::ColorEdit3("Ground", &m_atmosphereData.groundColor.x);
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

void SkyBox::GuiWeatherMap() {
    if (ImGui::CollapsingHeader("WEATHER MAP SETTINGS", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();

        // [ 분포 및 기본 설정 ]
        ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.5f, 1.0f), "[ Base Density ]");
        ImGui::SliderFloat("Cloud Coverage", &m_cloudMapData.coverage, 0.0f, 1.0f);

        // [ 노이즈 디테일 ]
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.5f, 1.0f), "[ Noise Frequency ]");
        ImGui::DragFloat("Perlin Freq", &m_cloudMapData.perlinFbmFreq, 0.1f, 1.0f, 20.0f);
        ImGui::SliderFloat("Perlin Octaves", &m_cloudMapData.perlinOctaves, 1.0f, 8.0f, "%.0f");
        ImGui::SliderFloat("Perlin Bias", &m_cloudMapData.perlinBias, 0.0f, 1.0f);
        ImGui::DragFloat("Worley Freq", &m_cloudMapData.worleyFbmFreq, 0.1f, 1.0f, 20.0f);

        // [ 바람 설정 ]
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.5f, 1.0f), "[ Wind Movement ]");
        ImGui::SliderFloat2("Wind Direction", &m_cloudMapData.windDirection.x, -1.0f, 1.0f);
        ImGui::DragFloat("Wind Speed", &m_cloudMapData.windSpeed, 0.01f, 0.0f, 5.0f);

        // [ 결과물 미리보기 ]
        ImGui::Spacing();
        ImGui::Text("Weather Map Preview (R: Coverage, G: Type)");
        if (m_weatherMapRT->GetSRV()) {
            // 256x256 사이즈로 미리보기 출력
            ImGui::Image((ImTextureID)m_weatherMapRT->GetSRV(), ImVec2(256, 256));
        }

        ImGui::Unindent();
    }
} // GuiWeatherMap