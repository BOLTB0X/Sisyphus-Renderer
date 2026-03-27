#include "Pch.h"
#include "DepthRecorder.h"
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"

using namespace DirectX;
using namespace SharedConstants;
using namespace ConstantBuffer;

DepthRecorder::DepthRecorder() {
    m_prevMatrixBufferData.world = XMMatrixIdentity();
    m_prevMatrixBufferData.view = XMMatrixIdentity();
    m_prevMatrixBufferData.projection = XMMatrixIdentity();
} // DepthRecorder

DepthRecorder::~DepthRecorder() {
} // ~DepthRecorder

bool DepthRecorder::Init(const InitParams& params) {
    if (!params.device || !params.hwnd) return false;

    return InitShader(params.device, params.hwnd);
} // Init

bool DepthRecorder::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    if (!context) return false;

    if (!UpdateMatrixBuffer(context, params.worldMatrix, params.viewMatrix, params.projectionMatrix)) {
        return false;
    }

    context->VSSetConstantBuffers(0, 1, m_matrixBuffer.GetAddressOf());
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(nullptr, nullptr, 0);

    return true;
} // Render

bool DepthRecorder::InitShader(ID3D11Device* device, HWND hwnd) {
    using namespace ShaderHelper;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    if (!InitVertexShader(device, hwnd, PathConstants::DEPTH_VS,
        layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<MatrixBuffer>(device, m_matrixBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // InitShader

bool DepthRecorder::UpdateMatrixBuffer(ID3D11DeviceContext* context,
    const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection) {

    MatrixBuffer data;
    data.world = XMMatrixTranspose(world);
    data.view = XMMatrixTranspose(view);
    data.projection = XMMatrixTranspose(projection);

    if (memcmp(&m_prevMatrixBufferData, &data, sizeof(MatrixBuffer)) == 0) {
        return true;
    }

    if (!ShaderHelper::UpdateConstantBuffer(context, m_matrixBuffer.Get(), data)) {
        return false;
    }

    m_prevMatrixBufferData = data;
    return true;
} // UpdateMatrixBuffer