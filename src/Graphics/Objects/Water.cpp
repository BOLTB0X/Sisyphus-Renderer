#include "Pch.h"
#include "Water.h"
#include "Resources/DefaultMesh.h"
// Utils
#include "Helpers/ShaderHelper.h"
#include "Helpers/DebugHelper.h"
#include "SharedConstants/PathConstants.h"
// define
#define BUFFER_SLOT_WORLD      2
#define BUFFER_SLOT_RESOLUTION 2
#define BUFFER_SLOT_WATER      3
#define BUFFER_SLOT_REFLECTION 4
#define SAMPLER_SLOT           0
#define TEX_NOR_WATER_SLOT     1
#define TEX_WAVE_WATER_SLOT    2
#define TEX_FLOW_MAP_SLOT      3
#define TEX_DEPTH_SLOT         4
#define TEX_REFLECTION_SLOT    5
#define TEX_SCENE_SLOT         6

using namespace DirectX;
using namespace ConstantBuffer;
using namespace ShaderHelper;
using namespace SharedConstants;

Water::Water() {
    m_waterMesh = std::make_unique<DefaultMesh>();
    m_reflectView = XMMatrixIdentity();
    m_waterNormalSRV = nullptr;
    m_waterWaveNormalSRV = nullptr;
    m_flowSRV = nullptr;
    m_linearSampler = nullptr;
    temp = nullptr;
} // Water

Water::~Water() {
    m_waterNormalSRV = nullptr;
    m_waterWaveNormalSRV = nullptr;
    m_flowSRV = nullptr;
    m_linearSampler = nullptr;
    temp = nullptr;
} // ~Water

bool Water::Init(const InitParams& params) {
    if (!params.device) {
        return false;
    }

    if (!m_waterMesh->Init(params.device, 1, DefaultMesh::DefaultMeshType::Quad)) {
        return false;
    }

    m_waterData.waterHeight = params.waterHeight;
    m_waterNormalSRV = params.waterNormalSRV;
    m_waterWaveNormalSRV = params.waterWaveNormalSRV;
    m_flowSRV = params.flowSRV;
    m_linearSampler = params.linearWrapSampler;

    if (!InitShader(params.device, params.hwnd, params.screenWidth, params.screenHeight)) {
        DebugHelper::DebugPrint("InitShader 초기화 실패");
        return false;
    }

    return true;
} // Init

void Water::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    float camX = params.cameraPosition.x;
    float camZ = params.cameraPosition.z;
    float waterScale = 5000.0f;
    XMMATRIX scale = XMMatrixScaling(waterScale, waterScale, 1.0f);
    XMMATRIX rot = XMMatrixRotationX(XMConvertToRadians(90.0f));
    XMMATRIX trans = XMMatrixTranslation(camX, -m_waterData.waterHeight, camZ);
    XMMATRIX world = scale * rot * trans;

    m_worldData.world = XMMatrixTranspose(world);

    XMStoreFloat4x4(&m_reflectionMatrix.reflectView, XMMatrixTranspose(m_reflectView));

    if (!UpdateConstantBuffer(context, m_worldBuffer.Get(), m_worldData) ||
        !UpdateConstantBuffer(context, m_reflectionMatrixBuffer.Get(), m_reflectionMatrix) ||
        !UpdateConstantBuffer(context, m_resolutionBuffer.Get(), m_resolutionData) ||
        !UpdateConstantBuffer(context, m_waterBuffer.Get(), m_waterData)) {
        return;
    }

    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    context->PSSetSamplers(SAMPLER_SLOT, 1, &m_linearSampler);

    context->VSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_worldBuffer.GetAddressOf());
    context->VSSetConstantBuffers(BUFFER_SLOT_REFLECTION, 1, m_reflectionMatrixBuffer.GetAddressOf());

    context->PSSetConstantBuffers(BUFFER_SLOT_RESOLUTION, 1, m_resolutionBuffer.GetAddressOf());
    context->PSSetConstantBuffers(BUFFER_SLOT_WATER, 1, m_waterBuffer.GetAddressOf());

    context->PSSetShaderResources(TEX_NOR_WATER_SLOT, 1, &m_waterNormalSRV);
    context->PSSetShaderResources(TEX_WAVE_WATER_SLOT, 1, &m_waterWaveNormalSRV);
    context->PSSetShaderResources(TEX_FLOW_MAP_SLOT, 1, &m_flowSRV);
    context->PSSetShaderResources(TEX_DEPTH_SLOT, 1, &params.sceneDepthSRV);
    context->PSSetShaderResources(TEX_REFLECTION_SLOT, 1, &params.reflectionSRV);
    context->PSSetShaderResources(TEX_SCENE_SLOT, 1, &params.sceneSRV);

    temp = params.reflectionSRV;

    m_waterMesh->RenderBuffer(context);
    context->DrawIndexed(m_waterMesh->GetIndexCount(), 0, 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(TEX_NOR_WATER_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_WAVE_WATER_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_FLOW_MAP_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_DEPTH_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_REFLECTION_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_SCENE_SLOT, 1, &nullSRV);
} // Render

void Water::OnGui() {
    ImGui::Begin("Water Composite Control");

    ImGui::Text("Transform Settings");
    ImGui::SliderFloat("Water Height", &m_waterData.waterHeight, -500.0f, 500.0f);

    ImGui::Separator();

    ImGui::Text("Optical & Wave Settings");
    ImGui::SliderFloat("Distortion", &m_waterData.waterDistortion, 0.0f, 0.1f);
    ImGui::SliderFloat("Reflectivity", &m_waterData.reflectivity, 0.0f, 1.0f);
    ImGui::SliderFloat("Density", &m_waterData.density, 0.0f, 2.0f);

    ImGui::Separator();

    ImGui::Text("Color Settings");
    ImGui::ColorEdit3("Shallow Color", &m_waterData.waterColorShallow.x);
    ImGui::ColorEdit3("Deep Color", &m_waterData.waterColorDeep.x);

    if (temp) {
        ImGui::Text("Ref:");
        ImGui::Image((ImTextureID)temp, ImVec2(256, 256));
    }

    ImGui::End();
} // OnGui

void Water::SetReflectView(XMMATRIX mat) {
    m_reflectView = mat;
} // m_reflectView

float Water::GetWaterHeight() const { 
    return m_waterData.waterHeight;
} // GetWaterHeight

XMMATRIX Water::GetWorldMatrix() {
    return m_transform.GetWorldMatrix();
} // GetWorldMatrix

bool Water::InitShader(ID3D11Device* device, HWND hwnd, const int& screenWidth, const int& screenHeight) {
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    if (InitVertexShader(device, hwnd, PathConstants::WATER_VS,
        layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf()) == false) {
        DebugHelper::DebugPrint("Water VS 초기화 실패");
        return false;
    }

    if (InitPixelShader(device, hwnd, PathConstants::WATER_PS,
        m_pixelShader.GetAddressOf()) == false) {
        DebugHelper::DebugPrint("Water PS 초기화 실패");
        return false;
    }

    if (!InitConstantBuffer<WorldBuffer>(device, m_worldBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<ResolutionBuffer>(device, m_resolutionBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<WaterBuffer>(device, m_waterBuffer.GetAddressOf())) {
        DebugHelper::DebugPrint("WaterBuffer 초기화 실패");
        return false;
    }

    if (!InitConstantBuffer<ReflectionMatrixBuffer>(device, m_reflectionMatrixBuffer.GetAddressOf())) {
        return false;
    }

    m_resolutionData.resolution = XMFLOAT2(1.0f / screenWidth, 1.0f / screenHeight);

    return true;
} // InitShader