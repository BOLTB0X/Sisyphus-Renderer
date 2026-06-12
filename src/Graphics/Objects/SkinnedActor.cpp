#include "Pch.h"
#include "Objects/SkinnedActor.h"
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
#define TRANSFORM_OFFSET       10.0f
#define WORLD_BUFFER_SLOT      2
#define ANIMATION_BUFFER_SLOT  3
#define ABEDO_TEXTURE_SLOT     0
#define NORMAL_TEXTURE_SLOT    1

using namespace DirectX;
using namespace SharedConstants;
using namespace ConstantBuffer;

SkinnedActor::SkinnedActor() : AssimpModel(), ActorObject() {
    m_linerSampler = nullptr;
} // SkinnedActor

SkinnedActor::~SkinnedActor() {
    m_linerSampler = nullptr;
} // ~SkinnedActor

bool SkinnedActor::Init(const InitParams& params) {
    if (params.device == nullptr || params.context == nullptr) {
        return false;
    }

    m_textureMgr = params.textMgr;
    if (!AssimpModel::Init(params.device, params.context, m_textureMgr, params.path)) {
        return false;
    }
    else {
        SetScale(0.001f, 0.001f, 0.001f);
        SetPosition(-80.0f, 0.0f, -80.0f);
    }

    if (!InitShader(params.device, params.hwnd, params.VSPath, params.PSPath)) {
        return false;
    }

    m_linerSampler = params.linerSampler;

    if (m_modelType == AssimpModel::ModelType::Skinned) {
        m_Animator.Init(this);

        //for (const auto& clip : m_clips) {
        //    DebugHelper::DebugPrint("Clip: " + clip.name);
        //}

        m_Animator.Play(m_clips.empty() ? "" : m_clips[0].name);
    }

    return true;
} // Init

void SkinnedActor::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    auto BindTexture = [&](ID3D11ShaderResourceView* srv, UINT slot) {
        if (srv) {
            context->PSSetShaderResources(slot, 1, &srv);
        }
        else {
            ID3D11ShaderResourceView* nullSRV = nullptr;
            context->PSSetShaderResources(slot, 1, &nullSRV);
        }
    }; // BindTexture

    if (!context) {
        return;
    }

    m_worldData.world = XMMatrixTranspose(params.world);
    if (!ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_worldData)) {
        DebugHelper::DebugPrint("world buffer 문제");
        return;
    }
    context->VSSetConstantBuffers(WORLD_BUFFER_SLOT, 1, m_worldBuffer.GetAddressOf());

    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_skinnedVertexShader.Get(), nullptr, 0);

    const auto& matrices = m_Animator.GetBoneMatrices();
    for (size_t i = 0; i < matrices.size() && i < 256; ++i) {
        m_boneData.boneMatrices[i] = XMMatrixTranspose(matrices[i]);
    }

    if (ShaderHelper::UpdateConstantBuffer(context, m_boneBuffer.Get(), m_boneData) == false) {
        return;
    }
    context->VSSetConstantBuffers(ANIMATION_BUFFER_SLOT, 1, m_boneBuffer.GetAddressOf());

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

        BindTexture(mat.albedo ? mat.albedo->GetSRV() : nullptr, ABEDO_TEXTURE_SLOT);
        BindTexture(mat.normal ? mat.normal->GetSRV() : nullptr, NORMAL_TEXTURE_SLOT);

        mesh->RenderBuffer(context);

    } // for
    m_RenderCount++;
} // Render

void SkinnedActor::RenderShadow(ID3D11DeviceContext* context, const RenderShadowParams& params) {
    if (!context || !params.shadowMap || !params.shadowParams || !params.states) {
        return;
    }

    const auto& matrices = m_Animator.GetBoneMatrices();
    for (size_t i = 0; i < matrices.size() && i < 256; ++i) {
        m_boneData.boneMatrices[i] = XMMatrixTranspose(matrices[i]);
    }

    if (ShaderHelper::UpdateConstantBuffer(context, m_boneBuffer.Get(), m_boneData) == false) {
        return;
    }
    context->VSSetConstantBuffers(ANIMATION_BUFFER_SLOT, 1, m_boneBuffer.GetAddressOf());

    for (const auto& mesh : m_meshes) {
        unsigned int matIndex = mesh->GetMaterialIndex();

        params.shadowParams->alphaSRV = m_materials[matIndex].albedo->GetSRV();

        context->RSSetState(params.states->GetCullNone());
        context->OMSetBlendState(params.states->GetBlendState(), nullptr, 0xFFFFFFFF);

        params.shadowMap->RenderTransparent(context, *params.shadowParams);
        mesh->RenderBuffer(context);
    }
} // RenderShadow

void SkinnedActor::Animate(float delta) {
    m_Animator.Update(delta);
} // SkinnedActor

void SkinnedActor::OnGui() {
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

XMMATRIX SkinnedActor::GetWorldMatrix() {
    XMMATRIX fixRotation = XMMatrixRotationY(XMConvertToRadians(90.0f));
    return fixRotation * m_transform.GetWorldMatrix();
} // GetWorldMatrix

bool SkinnedActor::InitShader(ID3D11Device* device, HWND hwnd, const std::wstring& vsPath, const std::wstring& psPath) {
    using namespace ShaderHelper;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (!InitVertexShader(device, hwnd, vsPath,
        layoutDesc, ARRAYSIZE(layoutDesc), m_skinnedVertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
        return false;
	}

    if (!InitPixelShader(device, hwnd, psPath, m_pixelShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<WorldBuffer>(device, m_worldBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<BoneBuffer>(device, m_boneBuffer.GetAddressOf())) {
        //DebugHelper::DebugPrint("CheckTransparentBuffer 문제");
        return false;
    }

    return true;
} // InitShader

