#include "Pch.h"
#include "Objects/Stone.h"
#include "Resources/PBRMesh.h"
#include "Resources/Texture.h"
#include "Resources/TextureManager.h"
#include "Resources/VertexTypes.h"
#include "Shaders/StoneShader.h"
// Utils
#include "SharedConstants/PathConstants.h"
// define
#define ABEDO_TEXTURE_SLOT     0
#define NORMAL_TEXTURE_SLOT    1
#define METALLIC_TEXTURE_SLOT  2
#define ROUGHNESS_TEXTURE_SLOT 3
#define AO_TEXTURE_SLOT        4

using namespace DirectX;
using namespace SharedConstants;

Stone::Stone() : AssimpModel() {
    m_shader = std::make_unique<StoneShader>();
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
;
    if (!m_shader->Init(device, hwnd, PathConstants::STONE_VS, PathConstants::STONE_PS)) {
        return false;
    }

    return true;
} // Init

void Stone::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    StoneShader::RenderParams shaderParams;
    shaderParams.world      = GetWorldMatrix();
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
    m_shader->SetSampler(sampler);
} // SetSampler

XMMATRIX Stone::GetWorldMatrix() {
    return m_transform.GetWorldMatrix();
} // GetWorldMatrix

unsigned int Stone::GetRenderCount() const {
    return m_RenderCount;
} // GetRenderCount