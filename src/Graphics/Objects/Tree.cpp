#include "Pch.h"
#include "Objects/Tree.h"
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
#define TRANSFORM_OFFSET       5.0f
#define ABEDO_TEXTURE_SLOT     0
#define NORMAL_TEXTURE_SLOT    1
#define SPECULAR_TEXTURE_SLOT  2
#define OPACITY_TEXTURE_SLOT   3
#define LIGHTING_TEXTURE_SLOT  4

using namespace DirectX;
using namespace SharedConstants;

Tree::Tree() : AssimpModel() {
    m_linerSampler = nullptr;
    m_transform = Transform();
    m_RenderCount = 0;
} // Tree

Tree::~Tree() {
    m_linerSampler = nullptr;
} // ~Stone

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

    SetPosition(50.0f, 0.0f, 50.0f);
    SetScale(TRANSFORM_OFFSET, TRANSFORM_OFFSET, TRANSFORM_OFFSET);
    return true;
} // Init

void Tree::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    RenderParams shaderParams;
    shaderParams.world = GetWorldMatrix();

    m_worldData.world = XMMatrixTranspose(params.world);
    if (!ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_worldData)) {
        DebugHelper::DebugPrint("Failed to update world buffer");
        return;
    }
    context->VSSetConstantBuffers(2, 1, m_worldBuffer.GetAddressOf());
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

    if (m_linerSampler) {
        context->PSSetSamplers(0, 1, &m_linerSampler);
    }

    for (const auto& mesh : m_meshes) {
        unsigned int matIndex = mesh->GetMaterialIndex();

        if (matIndex < m_materials.size()) {
            const auto& mat = m_materials[matIndex];

            if (mat.alpha != nullptr) {
                context->PSSetShader(m_twigPixelShader.Get(), nullptr, 0);
            }
            else {
                context->PSSetShader(m_barkPixelShader.Get(), nullptr, 0);
            }

            auto BindTexture = [&](ID3D11ShaderResourceView* srv, UINT slot) {
                if (srv) {
                    context->PSSetShaderResources(slot, 1, &srv);
                }
            };

            BindTexture(mat.albedo ? mat.albedo->GetSRV() : nullptr, ABEDO_TEXTURE_SLOT);
            BindTexture(mat.normal ? mat.normal->GetSRV() : nullptr, NORMAL_TEXTURE_SLOT);
            BindTexture(mat.specular ? mat.specular->GetSRV() : nullptr, SPECULAR_TEXTURE_SLOT);
            BindTexture(mat.alpha ? mat.alpha->GetSRV() : nullptr, OPACITY_TEXTURE_SLOT);
            BindTexture(mat.lighting ? mat.lighting->GetSRV() : nullptr, LIGHTING_TEXTURE_SLOT);
        }

        mesh->RenderBuffer(context);
    } // for
    m_RenderCount++;
} // Render

void Tree::DrawIndexed(ID3D11DeviceContext* context) {
    for (const auto& mesh : m_meshes) {
        mesh->BindBuffers(context);
        context->DrawIndexed(mesh->GetIndexCount(), 0, 0);
    }
} // DrawIndexed

void Tree::OnGui() {
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.5f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));

    // 모델 정보를 담은 메인 헤더
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

            // 각 머테리얼별 트리 노드
            if (ImGui::TreeNode((void*)(intptr_t)i, "Material [%d]: %s", (int)i, mat.name.c_str())) {

                ImGui::BeginGroup();

                auto ShowStatus = [](const char* type, bool isLoaded) {
                    ImGui::Text("%-10s:", type);
                    ImGui::SameLine();
                    if (isLoaded) {
                        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), " [ LOADED ]");
                    }
                    else {
                        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), " [ MISSING ]");
                    }
                    };
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
    return m_transform.GetWorldMatrix();
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

    if (!InitPixelShader(device, hwnd, PathConstants::TREE_BARK_PS, m_barkPixelShader.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(device, hwnd, PathConstants::TREE_TWIG_PS, m_twigPixelShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<WorldBuffer>(device, m_worldBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // InitShader
