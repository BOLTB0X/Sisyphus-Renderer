#include "Pch.h"
#include "Grass.h"
#include "Components/Frustum.h"
// Utils
#include "Helpers/ShaderHelper.h"
#include "Helpers/DebugHelper.h"
#include "SharedConstants/PathConstants.h"
// define
#define GRASS_BUFFER_SLOT  3
#define GRASS_TEXTURE_SLOT 0
#define GRASS_SAMPLER_SLOT 0
#define QUAD_VERTEX_COUNT  4

using namespace DirectX;
using namespace ShaderHelper;
using namespace SharedConstants;

Grass::Grass() {
    m_grassSRV = nullptr;
	m_linearSampler = nullptr;
    m_prevGrassData.padding.x = -1.0f;
} // Grass

Grass::~Grass() {
    m_grassSRV = nullptr;
	m_linearSampler = nullptr;    
} // ~Grass

bool Grass::Init(const InitParams& params) {
    if (!params.device || !params.hwnd || !params.grass) {
        return false;
    }
    
    m_grassSRV = params.grass;

    return InitShader(params.device, params.hwnd);
} // Init

void Grass::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    if (!context || !params.visibleNodes) {
        return;
    }

    if (UpdateGrassBuffer(context)) {
        context->GSSetConstantBuffers(GRASS_BUFFER_SLOT, 1, m_grassBuffer.GetAddressOf());
        context->PSSetConstantBuffers(GRASS_BUFFER_SLOT, 1, m_grassBuffer.GetAddressOf());
    }

    context->IASetInputLayout(m_layout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->GSSetShader(m_geometryShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    context->PSSetShaderResources(GRASS_TEXTURE_SLOT, 1, &m_grassSRV);
	context->PSSetSamplers(GRASS_SAMPLER_SLOT, 1, &m_linearSampler);

    UINT stride = sizeof(QuadTree::GrassSeed);
    UINT offset = 0;

    for (auto* node : *params.visibleNodes) {
        if (!node->grassSeedVertexBuffer.Get() || node->grassSeedCount == 0) {
            continue;
        }

        context->IASetVertexBuffers(0, 1, node->grassSeedVertexBuffer.GetAddressOf(), &stride, &offset);
        context->Draw(node->grassSeedCount, 0);
    }

    context->GSSetShader(nullptr, nullptr, 0);
} // Render

void Grass::RenderFar(ID3D11DeviceContext* context, const RenderParams& params) {
    if (!context || !params.visibleNodes) {
        return;
    }

    if (UpdateGrassBuffer(context)) {
        context->VSSetConstantBuffers(GRASS_BUFFER_SLOT, 1, m_grassBuffer.GetAddressOf());
        context->PSSetConstantBuffers(GRASS_BUFFER_SLOT, 1, m_grassBuffer.GetAddressOf());
    }

    context->IASetInputLayout(m_farLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    context->VSSetShader(m_farVertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_farPixelShader.Get(), nullptr, 0);

    context->PSSetShaderResources(GRASS_TEXTURE_SLOT, 1, &m_grassSRV);
    context->PSSetSamplers(GRASS_SAMPLER_SLOT, 1, &m_linearSampler);

    ID3D11Buffer* nullVB = nullptr;
    UINT nullStride = 0, nullOffset = 0;
    context->IASetVertexBuffers(0, 1, &nullVB, &nullStride, &nullOffset);

    UINT stride = sizeof(QuadTree::GrassFarInstance);
    UINT offset = 0;

    for (auto* node : *params.visibleNodes) {
        if (!node->grassFarInstanceBuffer.Get() || node->grassFarInstanceCount == 0)
        {
            continue;
        }

        context->IASetVertexBuffers(1, 1,
            node->grassFarInstanceBuffer.GetAddressOf(), &stride, &offset);

        // 4버텍스 × N인스턴스
        context->DrawInstanced(QUAD_VERTEX_COUNT, node->grassFarInstanceCount, 0, 0);
    }
} // RenderFar

void Grass::OnGui() {
    ImGui::Begin("Grass Control");

    ImGui::SliderFloat("Blade Width", &m_grassData.width, 0.1f, 10.0f);
    ImGui::SliderFloat("Blade Height", &m_grassData.height, 0.1f, 20.0f);
    ImGui::SliderFloat("Wind Strength", &m_grassData.windStrength, 0.0f, 2.0f);
    ImGui::SliderFloat("Wind Speed", &m_grassData.windSpeed, 0.0f, 5.0f);

    if (ImGui::Button("Reset")) {
        m_grassData = GrassBuffer();
    }

    ImGui::End();
} // OnGui

bool Grass::InitShader(ID3D11Device* device, HWND hwnd) {
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0,
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "HEIGHT",   0, DXGI_FORMAT_R32_FLOAT,        0, 12,
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,     0, 16,
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (InitVertexShader(device, hwnd, PathConstants::GRASS_VS,
        layoutDesc, ARRAYSIZE(layoutDesc),
        m_vertexShader.GetAddressOf(), m_layout.GetAddressOf()) == false) {
        DebugHelper::DebugPrint("Grass VS 초기화 실패");
        return false;
    }

    if (InitGeometryShader(device, hwnd, PathConstants::GRASS_GS,
        m_geometryShader.GetAddressOf()) == false) {
        DebugHelper::DebugPrint("Grass GS 초기화 실패");
        return false;
    }

    if (InitPixelShader(device, hwnd, PathConstants::GRASS_PS,
        m_pixelShader.GetAddressOf()) == false) {
        DebugHelper::DebugPrint("Grass PS 초기화 실패");
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC farLayoutDesc[] = {
        { "INSTANCE_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
          1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "INSTANCE_SCALE",    0, DXGI_FORMAT_R32_FLOAT,
          1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "INSTANCE_UV",       0, DXGI_FORMAT_R32G32_FLOAT,
          1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "INSTANCE_PADDING",  0, DXGI_FORMAT_R32G32_FLOAT,
          1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };

    if (InitVertexShader(device, hwnd, PathConstants::GRASS_FAR_VS,
        farLayoutDesc, ARRAYSIZE(farLayoutDesc),
        m_farVertexShader.GetAddressOf(), m_farLayout.GetAddressOf()) == false) {
        DebugHelper::DebugPrint("Grass Far VS 초기화 실패");
        return false;
    }

    if (InitPixelShader(device, hwnd, PathConstants::GRASS_FAR_PS,
        m_farPixelShader.GetAddressOf()) == false) {
        DebugHelper::DebugPrint("Grass Far PS 초기화 실패");
        return false;
    }

    if (!ShaderHelper::InitConstantBuffer<GrassBuffer>(device, m_grassBuffer.GetAddressOf())) {
        DebugHelper::DebugPrint("Grass Constant Buffer 초기화 실패");
        return false;
    }

    return true;
} // InitShader

bool Grass::UpdateGrassBuffer(ID3D11DeviceContext* context) {
    if (memcmp(&m_prevGrassData, &m_grassData, sizeof(GrassBuffer)) == 0) {
        return true;
    }
    if (!ShaderHelper::UpdateConstantBuffer(context, m_grassBuffer.Get(), m_grassData)) {
        return false;
    }

    m_prevGrassData = m_grassData;
    return true;
} // UpdateGrassBuffer