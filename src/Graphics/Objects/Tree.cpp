#include "Pch.h"
#include "Objects/Tree.h"
// D3D11
#include "D3D11/D3D11State.h"
// Components
#include "Components/TextureManager.h"
// Resources
#include "Resources/PBRMesh.h"
#include "Resources/Texture.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "Helpers/ShaderHelper.h"
#include "Helpers/DebugHelper.h"
// define
#define TRANSFORM_OFFSET       20.0f
#define WORLD_BUFFER_SLOT      2
#define CHECK_LEAF_BUFFER_SLOT 3
#define ABEDO_TEXTURE_SLOT     0
#define NORMAL_TEXTURE_SLOT    1
#define ROUGHNESS_TEXTURE_SLOT 2
#define OPACITY_TEXTURE_SLOT   3
#define SSS_TEXTURE_SLOT       4

using namespace DirectX;
using namespace SharedConstants;

Tree::Tree() : AssimpModel() {
    m_linerSampler = nullptr;
    m_transform = Transform();
    m_RenderCount = 0;
	m_checkTranspData = CheckTransparentBuffer();
    m_leafKeywords = { "ClusterB", "ClusterB2" };
} // Tree

Tree::~Tree() {
    m_linerSampler = nullptr;
} // ~Tree

bool Tree::Init(const InitParams& params) {
    if (params.device == nullptr || params.context == nullptr) {
        return false;
    }

    m_textureMgr = params.textMgr;
    if (!AssimpModel::Init(params.device, params.context, m_textureMgr, params.path)) {
        return false;
    }

    if (!InitShader(params.device, params.hwnd)) {
        return false;
    }

    m_linerSampler = params.linerSampler;

    SetPosition(70.0f, 0.0f, 50.0f);
    SetScale(TRANSFORM_OFFSET, TRANSFORM_OFFSET, TRANSFORM_OFFSET);
    return true;
} // Init

void Tree::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    auto BindTexture = [&](ID3D11ShaderResourceView* srv, UINT slot) {
        if (srv) {
            context->PSSetShaderResources(slot, 1, &srv);
        }
        else {
            ID3D11ShaderResourceView* nullSRV = nullptr;
            context->PSSetShaderResources(slot, 1, &nullSRV);
        }
    };

    if (!context || !params.states) {
        return;
	}

    m_worldData.world = XMMatrixTranspose(params.world);
    if (!ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_worldData)) {
        DebugHelper::DebugPrint("world buffer 문제");
        return;
    }

    context->VSSetConstantBuffers(WORLD_BUFFER_SLOT, 1, m_worldBuffer.GetAddressOf());
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

    if (m_linerSampler) {
        context->PSSetSamplers(0, 1, &m_linerSampler);
    }

    for (const auto& mesh : m_meshes) {
        unsigned int matIndex = mesh->GetMaterialIndex();

        if (matIndex >= m_materials.size()) {
            continue;
        }
        
        const auto& mat = m_materials[matIndex];
        context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
        
        m_checkTranspData.isLeaf = IsTransparentMaterial(mat.name) ? 1 : 0;
        if (!ShaderHelper::UpdateConstantBuffer(context, m_checkLeafBuffer.Get(), m_checkTranspData)) {
            //DebugHelper::DebugPrint("투과 buffer 문제");
            return;
        }
        
        context->PSSetConstantBuffers(CHECK_LEAF_BUFFER_SLOT, 1, m_checkLeafBuffer.GetAddressOf());
        
        if (m_checkTranspData.isLeaf) {
            context->RSSetState(params.states->GetCullNone());
            context->OMSetBlendState(params.states->GetBlendState(), nullptr, 0xFFFFFFFF);
        }
        else {
            context->RSSetState(params.states->GetCullBackState());
            context->OMSetBlendState(params.states->GetNoBlendState(), nullptr, 0xFFFFFFFF);
        }

        BindTexture(mat.albedo ? mat.albedo->GetSRV() : nullptr, ABEDO_TEXTURE_SLOT);
        BindTexture(mat.normal ? mat.normal->GetSRV() : nullptr, NORMAL_TEXTURE_SLOT);
        BindTexture(mat.roughness ? mat.roughness->GetSRV() : nullptr, ROUGHNESS_TEXTURE_SLOT);
        BindTexture(mat.alpha ? mat.alpha->GetSRV() : nullptr, OPACITY_TEXTURE_SLOT);
        BindTexture(mat.subsurface ? mat.subsurface->GetSRV() : nullptr, SSS_TEXTURE_SLOT);
        mesh->RenderBuffer(context);

    } // for
    m_RenderCount++;
} // Render

void Tree::RenderShadow(ID3D11DeviceContext* context, const RenderShadowParams& params) {
    if (!context || !params.shadowMap || !params.shadowParams || !params.states) {
        return;
    }

    for (const auto& mesh : m_meshes) {
        unsigned int matIndex = mesh->GetMaterialIndex();
        if (matIndex >= m_materials.size()) continue;

        const auto& mat = m_materials[matIndex];
        bool isLeafMesh = IsTransparentMaterial(mat.name);

        params.shadowParams->alphaSRV = mat.alpha ? mat.alpha->GetSRV() : nullptr;

        if (isLeafMesh) {
            context->RSSetState(params.states->GetCullNone());
            context->OMSetBlendState(params.states->GetBlendState(), nullptr, 0xFFFFFFFF);

            params.shadowMap->RenderTransparent(context, *params.shadowParams);
        }
        else {
            context->RSSetState(params.states->GetCullBackState());
            context->OMSetBlendState(params.states->GetNoBlendState(), nullptr, 0xFFFFFFFF);

            params.shadowMap->RenderOpaque(context, *params.shadowParams);
        }

        mesh->RenderBuffer(context);
    }
} // RenderShadow

void Tree::OnGui() {
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.5f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));

    if (ImGui::CollapsingHeader("MODEL INSPECTOR", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PopStyleColor(3);

        ImGui::Indent();
        ImGui::Spacing();

        ImGui::TextDisabled("Resource Info");
        ImGui::Text("Total Meshes: "); ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%d", GetMeshCount());

        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "PBR MATERIALS STATUS");
        ImGui::Spacing();

        std::vector<AssimpModel::MaterialInfo> materials = GetMaterialInfos();

        for (size_t i = 0; i < materials.size(); ++i) {
            const auto& mat = materials[i];

            if (ImGui::TreeNode((void*)(intptr_t)i, "Material [%d]: %s", (int)i, mat.name.c_str())) {

                ImGui::BeginGroup();

                auto ShowStatus = [](const char* type, bool isLoaded) {
                    ImGui::Text("%-12s:", type);
                    ImGui::SameLine();
                    if (isLoaded) {
                        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), " [ LOADED ]");
                    }
                    else {
                        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), " [ MISSING ]");
                    }
                    };

                ShowStatus("Albedo", mat.hasAlbedo);
                ShowStatus("Normal", mat.hasNormal);
                ShowStatus("Roughness", mat.hasRoughness);
                ShowStatus("Alpha", mat.hasAlpha);
                ShowStatus("hasSubsurface", mat.hasSubsurface);

                ImGui::EndGroup();
                ImGui::TreePop();
            }
        }

        ImGui::Unindent();
    }
    else {
        ImGui::PopStyleColor(3);
    }
} // OnGui

void Tree::SetPosition(const XMFLOAT3& pos) {
    m_transform.SetPosition(pos);
} // SetPosition

void Tree::SetPosition(float x, float y, float z) {
    m_transform.SetPosition(x, y, z);
} // SetPosition

void Tree::SetRotation(const XMFLOAT3& rot) {
    m_transform.SetRotation(rot);
} // SetRotation

void Tree::SetRotation(float x, float y, float z) {
    m_transform.SetRotation(x, y, z);
} // SetRotation

void Tree::SetScale(const XMFLOAT3& scale) {
    m_transform.SetScale(scale);
} // SetScale

void Tree::SetScale(float x, float y, float z) {
    m_transform.SetScale(x, y, z);
} // SetScale

void Tree::Translate(const XMFLOAT3& delta) {
    m_transform.Translate(delta);
} // Translate

void Tree::Translate(float x, float y, float z) {
    m_transform.Translate(x, y, z);
} // Translate

void Tree::Rotate(const XMFLOAT3& delta) {
    m_transform.Rotate(delta);
} // Rotate

void Tree::Rotate(float x, float y, float z) {
    m_transform.Rotate(x, y, z);
} // Rotate

XMFLOAT3 Tree::GetPosition() const {
    return m_transform.GetPosition();
} // GetPosition

XMMATRIX Tree::GetWorldMatrix() {
    XMMATRIX correction = XMMatrixRotationX(XMConvertToRadians(90.0f));
    return correction * m_transform.GetWorldMatrix();
} // GetWorldMatrix

unsigned int Tree::GetRenderCount() const {
    return m_RenderCount;
} // GetRenderCount

bool Tree::InitShader(ID3D11Device* device, HWND hwnd) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    if (!InitVertexShader(device, hwnd, PathConstants::PBR_VS,
        layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(device, hwnd, PathConstants::TREE_PS, m_pixelShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<WorldBuffer>(device, m_worldBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<CheckTransparentBuffer>(device, m_checkLeafBuffer.GetAddressOf())) {
        //DebugHelper::DebugPrint("CheckTransparentBuffer 문제");
        return false;
    }

    return true;
} // InitShader

bool Tree::IsTransparentMaterial(const std::string& materialName) const {
    for (const auto& keyword : m_leafKeywords) {
        if (materialName.find(keyword) != std::string::npos) {
            return true;
        }
    }
    return false;
} // IsTransparentMaterial