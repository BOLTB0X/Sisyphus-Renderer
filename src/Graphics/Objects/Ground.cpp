#include "Pch.h"
#include "Ground.h"
#include "Resources/DefaultMesh.h"
// utils
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"
#include "SharedConstants/BuffersConstants.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
// define
#define TEXTURE_SLOT       10
#define SAMPLER_SLOT       5
#define BUFFER_SLOT_WORLD  2
#define BUFFER_SLOT_GROUND 3
#define BUFFER_SLOT_SHADOW 4

using namespace DirectX;
using namespace SharedConstants;
using namespace PathConstants;
using namespace ConstantBuffer;

Ground::Ground() {
	m_mesh = std::make_unique<DefaultMesh>();
    m_transform = Transform();
	m_prevGoundData.padding1 = -1.0f;
	//m_prevCommonData.padding1 = -1.0f;
    m_prevShadowData.padding.x = -1.0f;

    m_shadowSRV = nullptr;
    m_shadowSampler = nullptr;
} // Ground

Ground::~Ground() {
    m_shadowSRV = nullptr;
    m_shadowSampler = nullptr;
} // ~Ground

bool Ground::Init(const InitParams& params) {
    if (params.device == nullptr) {
        return false;
    }

    if (!m_mesh->Init(params.device, 1, DefaultMesh::DefaultMeshType::Quad)) {
        return false;
    }

    if (!InitShader(params.device, params.hwnd)) {
        return false;
    }
    
    m_ShadowData.mapWidth = BuffersConstants::SHADOWMAP_WIDTH;
    m_ShadowData.mapHeight = BuffersConstants::SHADOWMAP_HEIGHT;
    m_ShadowData.spread = BuffersConstants::SPREAD;
    m_ShadowData.bias = BuffersConstants::BIAS;
    return true;
} // Init

void Ground::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    context->PSSetShaderResources(TEXTURE_SLOT, 1, &m_shadowSRV);
    context->PSSetSamplers(SAMPLER_SLOT, 1, &m_shadowSampler);

    float camX = params.cameraPosition.x;
    float camZ = params.cameraPosition.z;
    float groundScale = 500.0f;
    XMMATRIX scale = XMMatrixScaling(groundScale, groundScale, 1.0f);
    XMMATRIX rot = XMMatrixRotationX(XMConvertToRadians(90.0f));
    XMMATRIX trans = XMMatrixTranslation(camX, -1.0f, camZ);
    XMMATRIX world = scale * rot * trans;

    m_worldData.world = XMMatrixTranspose(world);
    if (!ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_worldData)) {
        DebugHelper::DebugPrint("Failed to update world buffer");
        return;
    }
    context->VSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_worldBuffer.GetAddressOf());
    context->PSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_worldBuffer.GetAddressOf());
    //if (UpdateCommonBuffer(context, world, params.view, params.projection, params.cameraPosition, params.lightDir, params.lightDiffuse)) {
    //    context->VSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_commonBuffer.GetAddressOf());
    //    context->PSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_commonBuffer.GetAddressOf());
    //}

    if (UpdateGroundBuffer(context)) {
        context->PSSetConstantBuffers(BUFFER_SLOT_GROUND, 1, m_groundBuffer.GetAddressOf());
    }

    if (UpdateShadowBuffer(context, world)) {
        context->PSSetConstantBuffers(BUFFER_SLOT_SHADOW, 1, m_shadowBuffer.GetAddressOf());
    }

    m_mesh->RenderBuffer(context);
    context->DrawIndexed(m_mesh->GetIndexCount(), 0, 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(TEXTURE_SLOT, 1, &nullSRV);
} // Render

void Ground::DrawIndexed(ID3D11DeviceContext* context) {
    m_mesh->RenderBuffer(context);
    context->DrawIndexed(m_mesh->GetIndexCount(), 0, 0);
} // DrawIndexed

void Ground::OnGui() {
    ImGui::Begin("Ground Control");

    ImGui::ColorEdit3("Dark Sand", (float*)&m_GoundData.darkSand);
    ImGui::ColorEdit3("Light Sand", (float*)&m_GoundData.lightSand);

    if (ImGui::Button("Reset Colors")) {
        m_GoundData = GroundBuffer();
    }

    ImGui::End();
} // OnGui

XMMATRIX Ground::GetWorldMatrix() {
    return m_transform.GetWorldMatrix();
} // GetWorldMatrix

void Ground::SetShadowMap(ID3D11ShaderResourceView* srv) {
    m_shadowSRV = srv;
} // SetShadowMap

void Ground::SetShadowSampler(ID3D11SamplerState* sampler) {
    m_shadowSampler = sampler;
} // SetShadowSampler

bool Ground::InitShader(ID3D11Device* device, HWND hwnd) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (!InitVertexShader(device, hwnd, PathConstants::GROUND_VS,
        layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(device, hwnd, PathConstants::GROUND_PS, m_pixelShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<WorldBuffer>(device, m_worldBuffer.GetAddressOf()) ||
        !InitConstantBuffer<GroundBuffer>(device, m_groundBuffer.GetAddressOf()) ||
        !InitConstantBuffer<ShadowBuffer>(device, m_shadowBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // InitShader

//bool Ground::UpdateCommonBuffer(ID3D11DeviceContext* context,
//    const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
//    const XMFLOAT3& camPos, const XMFLOAT3& lightDir, const XMFLOAT4& lightDiff) {
//    using namespace ShaderHelper;
//    using namespace ConstantBuffer;
//
//    CommonBuffer buffer;
//
//    // 행렬 전치
//    buffer.world = XMMatrixTranspose(world);
//    buffer.view = XMMatrixTranspose(view);
//    buffer.projection = XMMatrixTranspose(projection);
//    buffer.viewInv = XMMatrixTranspose(XMMatrixInverse(nullptr, view));
//    buffer.projInv = XMMatrixTranspose(XMMatrixInverse(nullptr, projection));
//    buffer.cameraPosition = camPos;
//
//    XMVECTOR ld = XMLoadFloat3(&lightDir);
//    if (XMVectorGetX(XMVector3Length(ld)) < 1e-6f) {
//        buffer.lightDirection = { 0.0f, -1.0f, 0.0f };
//    }
//    else {
//        XMStoreFloat3(&buffer.lightDirection, XMVector3Normalize(ld));
//    }
//
//    buffer.lightDiffuse = lightDiff;
//
//    if (memcmp(&m_prevCommonData, &buffer, sizeof(CommonBuffer)) == 0) {
//        return true;
//    }
//
//    if (!UpdateConstantBuffer(context, m_commonBuffer.Get(), buffer)) {
//        return false;
//    }
//
//    m_prevCommonData = buffer;
//    return true;
//} // UpdateCommonBuffer

bool Ground::UpdateGroundBuffer(ID3D11DeviceContext* context) {
    using namespace ShaderHelper;

    if (memcmp(&m_prevGoundData, &m_GoundData, sizeof(GroundBuffer)) == 0) {
        return true;
    }

    if (!UpdateConstantBuffer(context, m_groundBuffer.Get(), m_GoundData)) {
        return false;
    }

    m_prevGoundData = m_GoundData;
    return true;
} // UpdateGroundBuffer

bool Ground::UpdateShadowBuffer(ID3D11DeviceContext* context, const DirectX::XMMATRIX& world) {
    using namespace ShaderHelper;

    m_ShadowData.world = XMMatrixTranspose(world);

    if (memcmp(&m_prevShadowData, &m_ShadowData, sizeof(ShadowBuffer)) == 0) {
        return true;
    }

    if (!UpdateConstantBuffer(context, m_shadowBuffer.Get(), m_ShadowData)) {
        return false;
    }

    m_prevShadowData = m_ShadowData;
    return true;
} // UpdateShadowBuffer