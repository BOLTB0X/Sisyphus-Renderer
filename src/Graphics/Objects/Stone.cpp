#include "Pch.h"
#include "Objects/Stone.h"
#include "Resources/PBRMesh.h"
#include "Resources/Texture.h"
#include "Resources/VertexTypes.h"
#include "Shaders/StoneShader.h"
// Utils
#include "SharedConstants/PathConstants.h"

using namespace DirectX;
using namespace SharedConstants;

Stone::Stone() : AssimpModel() {
    m_shader = std::make_unique<StoneShader>();
    m_worldMatrix = XMMatrixIdentity();
    m_sampler = nullptr;
} // Stone

Stone::~Stone() {
    m_sampler = nullptr;
} // ~Stone

bool Stone::Init(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd, const std::string& path) {
    if (!AssimpModel::Init(device, context, path)) {
        return false;
    }
;
    if (!m_shader->Init(device, hwnd, PathConstants::STONE_VS, PathConstants::STONE_PS)) {
        return false;
    }

    return true;
}

void Stone::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    StoneShader::RenderParams shaderParams;
    shaderParams.world      = m_worldMatrix;
    shaderParams.view       = params.view;
    shaderParams.projection = params.projection;
    shaderParams.camPos     = params.camPos;
    shaderParams.diffuse    = params.diffuse;
    shaderParams.lightDir   = params.lightDir;

    if (!m_shader->Render(context, shaderParams)) {
        return;
    }

    for (const auto& mesh : m_meshes) {
        unsigned int matIndex = mesh->GetMaterialIndex();

        if (matIndex < m_materials.size()) {
            const auto& mat = m_materials[matIndex];

            if (mat.albedo) {
                ID3D11ShaderResourceView* srv = mat.albedo->GetSRV();
                context->PSSetShaderResources(0, 1, &srv);
            }
            if (mat.normal) {
                ID3D11ShaderResourceView* srv = mat.normal->GetSRV();
                context->PSSetShaderResources(1, 1, &srv);
            }
            if (mat.metallic) {
                ID3D11ShaderResourceView* srv = mat.metallic->GetSRV();
                context->PSSetShaderResources(2, 1, &srv);
            }
            if (mat.roughness) {
                ID3D11ShaderResourceView* srv = mat.roughness->GetSRV();
                context->PSSetShaderResources(3, 1, &srv);
            }
            if (mat.ao) {
                ID3D11ShaderResourceView* srv = mat.ao->GetSRV();
                context->PSSetShaderResources(4, 1, &srv);
            }
        }

        mesh->RenderBuffer(context);
    } // for
} // Render

XMMATRIX Stone::GetWorldMatrix() const { 
    return m_worldMatrix; 
} // GetWorldMatrix

void Stone::SetPosition(float x, float y, float z) {
    m_worldMatrix = XMMatrixTranslation(x, y, z);
} // SetPosition

void Stone::SetSampler(ID3D11SamplerState* sampler) {
    m_shader->SetSampler(sampler);
} // SetSampler