#include "Pch.h"
#include "Water.h"
// D3D11
#include "D3D11/D3D11State.h"
// Components
#include "Components/TextureManager.h"
// Resources
#include "Resources/DefaultMesh.h"
#include "Resources/Texture.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "Helpers/ShaderHelper.h"
#include "Helpers/DebugHelper.h"
#define SAMPLER_SLOT          0
#define TEX_NOR_WATER_SLOT    1
#define TEX_WAVE_WATER_SLOT   2
#define TEX_FLOW_MAP_SLOT     3
#define TEX_DEPTH_SLOT        4
#define TEX_REFLECTION_SLOT   5
#define TEX_REFRACTION_SLOT   6
#define CONSTANS_SLOT1        2


using namespace SharedConstants;
using namespace ShaderHelper;
using namespace DirectX;
using namespace DebugHelper;

Water::Water() {
    m_waterMesh = std::make_unique<DefaultMesh>();
    m_waterBufferData = WaterBuffer();
    m_waterHeight = 0.0f;
    m_waterNormalSRV = nullptr;
    m_waterWaveNormalSRV = nullptr;
    m_flowSRV = nullptr;
    m_linearSampler = nullptr;
} // Water

Water::~Water() {
    m_waterNormalSRV = nullptr;
    m_waterWaveNormalSRV = nullptr;
    m_flowSRV = nullptr;
    m_linearSampler = nullptr;
} // ~Water

bool Water::Init(const InitParams& params) {
    if (params.device == nullptr) {
        return false;
    }

    m_waterHeight = params.waterHeight;
    m_waterNormalSRV = params.waterNormalSRV;
    m_waterWaveNormalSRV = params.waterWaveNormalSRV;
    m_flowSRV = params.flowSRV;
    m_linearSampler = params.linearWrapSampler;

    if (!m_waterMesh->Init(params.device,
        static_cast<UINT>(params.waterScale), DefaultMesh::DefaultMeshType::Quad)) {
        return false;
    }

    if (!InitShader(params.device, params.hwnd)) {
        return false;
    }

    return true;
} // Init

void Water::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2); // 90도 회전
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, m_waterHeight, 0.0f);

    m_waterBufferData.worldMatrix = DirectX::XMMatrixTranspose(rotation * translation);
    m_waterBufferData.waterHeight = m_waterHeight;

    if (!UpdateConstantBuffer(context, m_waterBuffer.Get(), m_waterBufferData)) {
        return;
    }
    else {
        context->VSSetConstantBuffers(CONSTANS_SLOT1, 1, m_waterBuffer.GetAddressOf());
        context->PSSetConstantBuffers(CONSTANS_SLOT1, 1, m_waterBuffer.GetAddressOf());
    }

    context->PSSetShaderResources(TEX_NOR_WATER_SLOT, 1, &m_waterNormalSRV);
    context->PSSetShaderResources(TEX_WAVE_WATER_SLOT, 1, &m_waterWaveNormalSRV);
    context->PSSetShaderResources(TEX_FLOW_MAP_SLOT, 1, &m_flowSRV);
    context->PSSetShaderResources(TEX_DEPTH_SLOT, 1, &params.sceneDepthSRV);
    context->PSSetShaderResources(TEX_REFLECTION_SLOT, 1, &params.reflectionSRV);
    context->PSSetShaderResources(TEX_REFRACTION_SLOT, 1, &params.refractionSRV);

    context->PSSetSamplers(SAMPLER_SLOT, 1, &m_linearSampler);

    m_waterMesh->RenderBuffer(context);
    context->DrawIndexed(m_waterMesh->GetIndexCount(), 0, 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(TEX_NOR_WATER_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_WAVE_WATER_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_FLOW_MAP_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_DEPTH_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_REFLECTION_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_REFRACTION_SLOT, 1, &nullSRV);
} // Render

float Water::GetWaterHeight() {
    return m_waterHeight;
} // GetWaterHeight

bool Water::InitShader(ID3D11Device* device, HWND hwnd) {
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0,
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,     0, 16,
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (InitVertexShader(device, hwnd, PathConstants::WATER_VS,
        layoutDesc, ARRAYSIZE(layoutDesc),
        m_vertexShader.GetAddressOf(), m_layout.GetAddressOf()) == false) {
        DebugHelper::DebugPrint("Water VS 초기화 실패");
        return false;
    }

    if (InitPixelShader(device, hwnd, PathConstants::WATER_PS,
        m_pixelShader.GetAddressOf()) == false) {
        DebugHelper::DebugPrint("Water PS 초기화 실패");
        return false;
    }

    if (!ShaderHelper::InitConstantBuffer<WaterBuffer>(device, m_waterBuffer.GetAddressOf())) {
        DebugHelper::DebugPrint("Water Constant Buffer 초기화 실패");
        return false;
    }

    return true;
} // InitShader