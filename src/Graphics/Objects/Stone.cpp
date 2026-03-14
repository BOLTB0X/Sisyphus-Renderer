#include "Pch.h"
#include "Objects/Stone.h"
#include "Resources/PBRMesh.h"
#include "Resources/Texture.h"
#include "Resources/TextureManager.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "Helpers/ShaderHelper.h"
// define
#define ABEDO_TEXTURE_SLOT     0
#define NORMAL_TEXTURE_SLOT    1
#define METALLIC_TEXTURE_SLOT  2
#define ROUGHNESS_TEXTURE_SLOT 3
#define AO_TEXTURE_SLOT        4

using namespace DirectX;
using namespace SharedConstants;

Stone::Stone() : AssimpModel() {
    m_sampler = nullptr;
	m_transform = Transform();
    m_RenderCount = 0;
} // Stone

Stone::~Stone() {
    m_sampler = nullptr;
} // ~Stone

bool Stone::Init(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd,
    std::shared_ptr<TextureManager> textureMgr, const std::string& path) {
    m_textureMgr = textureMgr;
    if (!AssimpModel::Init(device, context, m_textureMgr, path)) {
        return false;
    }

    if (!InitShader(device, hwnd)) {
        return false;
	}

    return true;
} // Init

void Stone::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    RenderParams shaderParams;
    shaderParams.world      = GetWorldMatrix();
    shaderParams.view       = params.view;
    shaderParams.projection = params.projection;
    shaderParams.camPos     = params.camPos;
    shaderParams.diffuse    = params.diffuse;
    shaderParams.lightDir   = params.lightDir;

    if (!RenderShader(context, params)) {
        return;
	}

    for (const auto& mesh : m_meshes) {
        unsigned int matIndex = mesh->GetMaterialIndex();

        if (matIndex < m_materials.size()) {
            const auto& mat = m_materials[matIndex];

            auto BindTexture = [&](ID3D11ShaderResourceView* srv, UINT slot) {
                if (srv) context->PSSetShaderResources(slot, 1, &srv);
            };

            BindTexture(mat.albedo ? mat.albedo->GetSRV() : nullptr, ABEDO_TEXTURE_SLOT);
            BindTexture(mat.normal ? mat.normal->GetSRV() : nullptr, NORMAL_TEXTURE_SLOT);
            BindTexture(mat.metallic ? mat.metallic->GetSRV() : nullptr, METALLIC_TEXTURE_SLOT);
            BindTexture(mat.roughness ? mat.roughness->GetSRV() : nullptr, ROUGHNESS_TEXTURE_SLOT);
            BindTexture(mat.ao ? mat.ao->GetSRV() : nullptr, AO_TEXTURE_SLOT);
        }

        mesh->RenderBuffer(context);
    } // for
    m_RenderCount++;
} // Render

void Stone::SetPosition(const XMFLOAT3& pos) {
    m_transform.SetPosition(pos);
} // SetPosition

void Stone::SetPosition(float x, float y, float z) {
    m_transform.SetPosition(x, y, z);
} // SetPosition

void Stone::SetRotation(const XMFLOAT3& rot) {
    m_transform.SetRotation(rot);
} // SetRotation

void Stone::SetRotation(float x, float y, float z) {
    m_transform.SetRotation(x, y, z);
} // SetRotation

void Stone::SetScale(const XMFLOAT3& scale) {
    m_transform.SetScale(scale);
} // SetScale

void Stone::SetScale(float x, float y, float z) {
    m_transform.SetScale(x, y, z);
} // SetScale

void Stone::Translate(const XMFLOAT3& delta) {
    m_transform.Translate(delta);
} // Translate

void Stone::Translate(float x, float y, float z) {
    m_transform.Translate(x, y, z);
} // Translate

void Stone::Rotate(const XMFLOAT3& delta) {
    m_transform.Rotate(delta);
} // Rotate

void Stone::Rotate(float x, float y, float z) {
    m_transform.Rotate(x, y, z);
} // Rotate

void Stone::SetSampler(ID3D11SamplerState* sampler) {
	m_sampler = sampler;
} // SetSampler

XMMATRIX Stone::GetWorldMatrix() {
    return m_transform.GetWorldMatrix();
} // GetWorldMatrix

unsigned int Stone::GetRenderCount() const {
    return m_RenderCount;
} // GetRenderCount

bool Stone::InitShader(ID3D11Device* device, HWND hwnd) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    if (!InitVertexShader(device, hwnd, PathConstants::STONE_VS,
        layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(device, hwnd, PathConstants::STONE_PS, m_pixelShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<MatCameraBuffer>(device, m_cameraBuffer.GetAddressOf())
        || !InitConstantBuffer<LightBuffer>(device, m_lightBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // InitShader

bool Stone::UpdateCameraBuffer(ID3D11DeviceContext* context,
    const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, const XMFLOAT3& camPos) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;

    MatCameraBuffer buffer;
    buffer.world = XMMatrixTranspose(world);
    buffer.view = XMMatrixTranspose(view);
    buffer.projection = XMMatrixTranspose(projection);
	buffer.cameraPosition = camPos;


    if (memcmp(&m_prevCameraData, &buffer, sizeof(MatCameraBuffer)) == 0) {
        return true;
    }
    if (!UpdateConstantBuffer(context, m_cameraBuffer.Get(), buffer)) {
        return false;
    }

    m_prevCameraData = buffer;
    return true;
} // UpdateMatrixBuffer

bool Stone::UpdateLightBuffer(ID3D11DeviceContext* context,
    const XMFLOAT4& diffuse, const XMFLOAT3& lightDir) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;

    LightBuffer buffer;
    buffer.diffuseColor = diffuse;
    buffer.lightDirection = lightDir;

    if (memcmp(&m_prevLightData, &buffer, sizeof(LightBuffer)) == 0) {
        return true;
    }
    if (!UpdateConstantBuffer(context, m_lightBuffer.Get(), buffer)) {
        return false;
    }

    m_prevLightData = buffer;
    return true;
} // UpdateLightBuffer

bool Stone::RenderShader(ID3D11DeviceContext* context, const RenderParams& params) {
    if (!UpdateCameraBuffer(context, params.world, params.view, params.projection, params.camPos))
        return false;
    if (!UpdateLightBuffer(context, params.diffuse, params.lightDir))
        return false;

    context->VSSetConstantBuffers(0, 1, m_cameraBuffer.GetAddressOf());
    context->PSSetConstantBuffers(1, 1, m_lightBuffer.GetAddressOf());
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    if (m_sampler) {
        context->PSSetSamplers(0, 1, &m_sampler);
    }
    return true;
} // RenderShader