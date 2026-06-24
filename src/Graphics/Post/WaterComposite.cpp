#include "Pch.h"
#include "WaterComposite.h"
// Data
#include "Data/RenderTexture.h"
// D3D11
#include "D3D11/D3D11State.h"
// Components
#include "Components/TextureManager.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "Helpers/ShaderHelper.h"
#include "Helpers/DebugHelper.h"
// define
#define SAMPLER_SLOT          0
#define TEX_NOR_WATER_SLOT    1
#define TEX_WAVE_WATER_SLOT   2
#define TEX_DEPTH_SLOT        3
#define TEX_SCENE_SLOT        4
#define TEX_NORMAL_SLOT       5
#define CONSTANS_SLOT1        2
#define CONSTANS_SLOT2        3

using namespace SharedConstants;
using namespace ShaderHelper;
using namespace DirectX;
using namespace DebugHelper;
using namespace ConstantBuffer;

WaterComposite::WaterComposite() {
    m_compositeRT = std::make_unique<RenderTexture>();
    m_waterBufferData = WaterBuffer();
    m_waterHeight = 0.0f;
    m_waterNormalSRV = nullptr;
    m_waterWaveNormalSRV = nullptr;
    m_linearSampler = nullptr;
} // WaterComposite

WaterComposite::~WaterComposite() {
    m_waterNormalSRV = nullptr;
    m_waterWaveNormalSRV = nullptr;
    m_linearSampler = nullptr;
} // ~WaterComposite

bool WaterComposite::Init(const InitParams& params) {
    if (!params.device || !params.hwnd) {
        return false;
    }

    if (!m_compositeRT->Init(params.device, params.screenWidth, params.screenHeight,
        RenderTexture::RenderTextureType::Normal, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        DebugHelper::DebugPrint("m_compositeRT 초기화 실패");
        return false;
    }

    m_waterHeight = params.waterHeight;
    m_waterNormalSRV = params.waterNormalSRV;
    m_waterWaveNormalSRV = params.waterWaveNormalSRV;
    m_linearSampler = params.linearWrapSampler;

    if (!InitShader(params.device, params.hwnd, params.screenWidth, params.screenHeight)) {
        DebugHelper::DebugPrint("InitShader 초기화 실패");
        return false;
    }

    return true;
} // Init

void WaterComposite::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    m_waterBufferData.waterHeight = m_waterHeight;

    if (UpdateConstantBuffer(context, m_resolutionBuffer.Get(), m_resolutionBufferData)) {
        context->PSSetConstantBuffers(CONSTANS_SLOT1, 1, m_resolutionBuffer.GetAddressOf());
    }

    if (UpdateConstantBuffer(context, m_waterBuffer.Get(), m_waterBufferData)) {
        context->PSSetConstantBuffers(CONSTANS_SLOT2, 1, m_waterBuffer.GetAddressOf());
    }

    context->PSSetShaderResources(TEX_NOR_WATER_SLOT, 1, &m_waterNormalSRV);
    context->PSSetShaderResources(TEX_WAVE_WATER_SLOT, 1, &m_waterWaveNormalSRV);
    context->PSSetShaderResources(TEX_DEPTH_SLOT, 1, &params.sceneDepthSRV);

    context->PSSetShaderResources(TEX_SCENE_SLOT, 1, &params.sceneSRV);
    context->PSSetShaderResources(TEX_NORMAL_SLOT, 1, &params.normalSRV);

    context->PSSetSamplers(SAMPLER_SLOT, 1, &m_linearSampler);

    context->Draw(3, 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(TEX_NOR_WATER_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_WAVE_WATER_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_DEPTH_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_SCENE_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_NORMAL_SLOT, 1, &nullSRV);

} // Render

void WaterComposite::ClearRT(ID3D11DeviceContext* context) {
    m_compositeRT->Clear(context);
} // ClearRT

ID3D11Texture2D* WaterComposite::GetTexture() const {
    return m_compositeRT->GetTexture();
} // GetTexture

ID3D11RenderTargetView* WaterComposite::GetRTV() const {
    return m_compositeRT->GetRTV();
} // GetRTV

ID3D11ShaderResourceView* WaterComposite::GetSRV() const {
    return m_compositeRT->GetSRV();
} // GetSRV

float WaterComposite::GetWaterHeight() {
    return m_waterHeight;
} // GetWaterHeight

bool WaterComposite::InitShader(ID3D11Device* device, HWND hwnd, const int& screenWidth, const int& screenHeight) {
    if (InitVertexShader(device, hwnd, PathConstants::POST_VS,
        nullptr, 0, m_vertexShader.GetAddressOf(), nullptr) == false) {
        DebugHelper::DebugPrint("WaterComposite VS 초기화 실패");
        return false;
    }

    if (InitPixelShader(device, hwnd, PathConstants::WATER_POST_PS,
        m_pixelShader.GetAddressOf()) == false) {
        DebugHelper::DebugPrint("WaterComposite PS 초기화 실패");
        return false;
    }

    if (!InitConstantBuffer<ResolutionBuffer>(device, m_resolutionBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<WaterBuffer>(device, m_waterBuffer.GetAddressOf())) {
        DebugHelper::DebugPrint("WaterComposite Constant Buffer 초기화 실패");
        return false;
    }

    m_resolutionBufferData.resolution = XMFLOAT2(1.0f / screenWidth, 1.0f / screenHeight);

    return true;
} // InitShader

void WaterComposite::OnGui() {
    ImGui::Text("WaterComposite Parameters");
    ImGui::Separator();

    ImGui::SliderFloat("WaterComposite Height", &m_waterHeight, 0.0f, 150.0f, "%.1f");

    ImGui::Spacing();
    ImGui::Text("WaterComposite Colors");

    ImGui::ColorEdit3("Shallow Color", &m_waterBufferData.waterColorShallow.x);
    ImGui::ColorEdit3("Deep Color", &m_waterBufferData.waterColorDeep.x);

    ImGui::Spacing();
    ImGui::Text("Optical & Wave Settings");

    ImGui::SliderFloat("Distortion", &m_waterBufferData.distortion, 0.0f, 0.1f, "%.4f");

    ImGui::SliderFloat("Reflectivity", &m_waterBufferData.reflectivity, 0.0f, 1.0f, "%.2f");

    ImGui::SliderFloat("Density", &m_waterBufferData.density, 0.0f, 2.0f, "%.2f");


    ImGui::Spacing();

} // OnGui