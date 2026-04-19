#include "Pch.h"
#include "ShadowMap.h"
#include "RenderTexture.h"
#include "Helpers/ShaderHelper.h"
#include "Helpers/DebugHelper.h"
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ShadowConstants.h"

using namespace DirectX;
using namespace SharedConstants;
using namespace ConstantBuffer;
using namespace DebugHelper;
using namespace BuffersConstants;

ShadowMap::ShadowMap() {
    m_shadowRT = std::make_unique<RenderTexture>();
    m_shadowViewport = {};
    m_prevMatrixBufferData.world = XMMatrixIdentity();
    m_prevMatrixBufferData.view = XMMatrixIdentity();
    m_prevMatrixBufferData.projection = XMMatrixIdentity();
} // ShadowMap

ShadowMap::~ShadowMap() {
} // ~ShadowMap

bool ShadowMap::Init(const InitParams& params) {
    if (!params.device || !params.hwnd) {
        return false;
    }

    if (!m_shadowRT->Init(params.device,
        ShadowConstants::SHADOWMAP_WIDTH, ShadowConstants::SHADOWMAP_HEIGHT,
        RenderTexture::RenderTextureType::Depth)) {
        return false;
    }

    m_shadowViewport.Width = static_cast<float>(m_shadowRT->GetWidth());
    m_shadowViewport.Height = static_cast<float>(m_shadowRT->GetHeight());
    m_shadowViewport.MinDepth = 0.0f;
    m_shadowViewport.MaxDepth = 1.0f;

    return InitShader(params.device, params.hwnd);
} // Init

bool ShadowMap::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    if (!context) {
        return false;
    }

    if (!UpdateMatrixBuffer(context, params.worldMatrix, params.viewMatrix, params.projectionMatrix)) {
        return false;
    }

    context->VSSetConstantBuffers(0, 1, m_matrixBuffer.GetAddressOf());
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(nullptr, nullptr, 0);

    return true;
} // Render

void ShadowMap::ClearShadowDepth(ID3D11DeviceContext* context) {
    m_shadowRT->ClearDepth(context);
} // ClearShadowDepth

RenderTexture*            ShadowMap::GetShadowRT() const { return m_shadowRT.get(); }
const D3D11_VIEWPORT&     ShadowMap::GetViewport() const { return m_shadowViewport; }
ID3D11DepthStencilView*   ShadowMap::GetDSV() { return m_shadowRT->GetDSV(); } // GetDSV
ID3D11ShaderResourceView* ShadowMap::GetSRV() { return m_shadowRT->GetSRV(); } // GetSRV

bool ShadowMap::InitShader(ID3D11Device* device, HWND hwnd) {
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

bool ShadowMap::UpdateMatrixBuffer(ID3D11DeviceContext* context,
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