#include "Pch.h"
#include "Grass.h"
#include "Components/Frustum.h"
// Utils
#include "Helpers/ShaderHelper.h"
#include "Helpers/DebugHelper.h"
#include "SharedConstants/PathConstants.h"

using namespace DirectX;
using namespace ShaderHelper;
using namespace SharedConstants;

Grass::Grass() {
} // Grass

Grass::~Grass() {
} // ~Grass

bool Grass::Init(const InitParams& params) {
    if (!params.device || !params.hwnd) {
        return false;
    }

    return InitShader(params.device, params.hwnd);
} // Init

void Grass::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    if (!context || !params.visibleNodes) {
        return;
    }

    context->IASetInputLayout(m_layout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->GSSetShader(m_geometryShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    UINT stride = sizeof(QuadTree::GrassSeed);
    UINT offset = 0;

    for (auto* node : *params.visibleNodes) {
        if (!node->grassSeedVertexBuffer.Get() || node->grassSeedCount == 0)
            continue;

        context->IASetVertexBuffers(0, 1, node->grassSeedVertexBuffer.GetAddressOf(), &stride, &offset);
        context->Draw(node->grassSeedCount, 0);
    }

    context->GSSetShader(nullptr, nullptr, 0);
} // Render

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

    return true;
} // InitShader