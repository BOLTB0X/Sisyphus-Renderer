#include "Pch.h"
#include "RigidActor.h"
// Core
#include "Core/RenderQueue.h"
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
#define ABEDO_TEXTURE_SLOT     0
#define NORMAL_TEXTURE_SLOT    1
#define METALLIC_TEXTURE_SLOT  2
#define AO_TEXTURE_SLOT        3
#define SMOOTH_TEXTURE_SLOT    4
#define EMI_TEXTURE_SLOT       5

using namespace DirectX;
using namespace SharedConstants;
using namespace ConstantBuffer;

RigidActor::RigidActor() {
    m_linerSampler = nullptr;
} // RigidActor

RigidActor::~RigidActor() {
    m_linerSampler = nullptr;
} // ~RigidActor

bool RigidActor::Init(const InitParams& params) {
    if (params.device == nullptr || params.context == nullptr) {
        return false;
    }

    m_textureMgr = params.textMgr;
    if (!AssimpModel::Init(params.device, params.context, m_textureMgr, params.path)) {
        return false;
    }
    else {
        SetScale(0.1f, 0.1f, 0.1f);
        SetPosition(-80.0f, 0.0f, -80.0f);
    }

    if (!InitShader(params.device, params.hwnd, params.VSPath, params.PSPath)) {
        return false;
    }

    m_linerSampler = params.linerSampler;

    if (m_modelType == AssimpModel::ModelType::RigidAnimated) {
        m_Animator.Init(this);
        m_Animator.Play(m_clips.empty() ? "" : m_clips[0].name);
    }

    return true;
} // Init

void RigidActor::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    if (!context) return;

    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_staticVertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    if (m_linerSampler) {
        context->PSSetSamplers(0, 1, &m_linerSampler);
    }

    const auto& nodeTransforms = m_Animator.GetNodeTransforms();

    RenderNode(context, GetRootNode(), params.world, nodeTransforms);

    m_RenderCount++;
} // Render

void RigidActor::RenderNode(ID3D11DeviceContext* context, const AssimpModel::ModelNode* node, const XMMATRIX& parentWorld, const std::unordered_map<std::string, XMMATRIX>& nodeTransforms) {
    if (!node) return;

    XMMATRIX nodeGlobalTransform = node->transformation;
    auto it = nodeTransforms.find(node->name);
    if (it != nodeTransforms.end()) {
        nodeGlobalTransform = it->second;
    }

    XMMATRIX finalWorld = nodeGlobalTransform * parentWorld;

    // 월드 버퍼 업데이트
    m_worldData.world = XMMatrixTranspose(finalWorld);
    if (ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_worldData)) {
        context->VSSetConstantBuffers(WORLD_BUFFER_SLOT, 1, m_worldBuffer.GetAddressOf());
    }

    auto BindTexture = [&](ID3D11ShaderResourceView* srv, UINT slot) {
        if (srv) context->PSSetShaderResources(slot, 1, &srv);
        else { ID3D11ShaderResourceView* nullSRV = nullptr; context->PSSetShaderResources(slot, 1, &nullSRV); }
    };

    for (int meshIndex : node->meshIndices) {
        if (meshIndex >= m_meshes.size()) continue;

        const auto& mesh = m_meshes[meshIndex];
        unsigned int matIndex = mesh->GetMaterialIndex();

        if (matIndex < m_materials.size()) {
            const auto& mat = m_materials[matIndex];
            BindTexture(mat.albedo ? mat.albedo->GetSRV() : nullptr, ABEDO_TEXTURE_SLOT);
            BindTexture(mat.normal ? mat.normal->GetSRV() : nullptr, NORMAL_TEXTURE_SLOT);
            BindTexture(mat.metallic ? mat.metallic->GetSRV() : nullptr, METALLIC_TEXTURE_SLOT);
            BindTexture(mat.ao ? mat.ao->GetSRV() : nullptr, AO_TEXTURE_SLOT);
            BindTexture(mat.roughness ? mat.roughness->GetSRV() : nullptr, SMOOTH_TEXTURE_SLOT);
            BindTexture(mat.emissive ? mat.emissive->GetSRV() : nullptr, EMI_TEXTURE_SLOT);
        }

        mesh->RenderBuffer(context);
    }

    // 자식 노드 순회
    for (const auto& child : node->children) {
        RenderNode(context, child.get(), parentWorld, nodeTransforms);
    }
} // RenderNode

void RigidActor::RenderShadow(ID3D11DeviceContext* context, const RenderShadowParams& params) {
    if (!context || !params.shadowMap || !params.shadowParams || !params.states) {
        return;
    }

    const auto& nodeTransforms = m_Animator.GetNodeTransforms();
    RenderShadowNode(context, GetRootNode(), params.shadowParams->worldMatrix, nodeTransforms, params);
} // RenderShadow

void RigidActor::RenderShadowNode(ID3D11DeviceContext* context, const AssimpModel::ModelNode* node, const XMMATRIX& parentWorld, const std::unordered_map<std::string, XMMATRIX>& nodeTransforms, const RenderShadowParams& params) {
    if (!node) return;

    XMMATRIX nodeGlobalTransform = node->transformation;
    auto it = nodeTransforms.find(node->name);
    if (it != nodeTransforms.end()) {
        nodeGlobalTransform = it->second;
    }

    XMMATRIX finalWorld = nodeGlobalTransform * parentWorld;

    m_worldData.world = XMMatrixTranspose(finalWorld);
    ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_worldData);
    context->VSSetConstantBuffers(WORLD_BUFFER_SLOT, 1, m_worldBuffer.GetAddressOf());
    params.shadowParams->worldMatrix = finalWorld;

    for (int meshIndex : node->meshIndices) {
        if (meshIndex >= m_meshes.size()) continue;

        const auto& mesh = m_meshes[meshIndex];
        unsigned int matIndex = mesh->GetMaterialIndex();

        params.shadowParams->alphaSRV = m_materials[matIndex].albedo ? m_materials[matIndex].albedo->GetSRV() : nullptr;

        context->RSSetState(params.states->GetCullNone());
        context->OMSetBlendState(params.states->GetBlendState(), nullptr, 0xFFFFFFFF);

        params.shadowMap->RenderOpaque(context, *params.shadowParams);
        mesh->RenderBuffer(context);
    }

    for (const auto& child : node->children) {
        RenderShadowNode(context, child.get(), finalWorld, nodeTransforms, params);
    }
} // RenderShadowNode

void RigidActor::Submit(const SubmitParams& params) {
    if (!params.opaqueQueue) return;

    DirectX::XMVECTOR posVec = DirectX::XMLoadFloat3(&m_transform.GetPosition());
    DirectX::XMVECTOR camVec = DirectX::XMLoadFloat3(&params.cameraPosition);
    float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(posVec, camVec)));

    // 멤버 변수 갱신
    m_worldData.world = DirectX::XMMatrixTranspose(params.worldMatrix);

    for (size_t i = 0; i < m_meshes.size(); ++i) {
        PBRMesh* currentMesh = m_meshes[i].get();
        unsigned int matIndex = currentMesh->GetMaterialIndex();
        if (matIndex >= m_materials.size()) continue;

        RenderQueue::DrawCommand cmd;
        cmd.vs = m_staticVertexShader.Get();
        cmd.ps = m_pixelShader.Get();
        cmd.sortKey = GenerateSortKey(params.shaderID, static_cast<uint16_t>(matIndex), distance);

        cmd.execute = [this, currentMesh, matIndex](ID3D11DeviceContext* context) {
            if (ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_worldData)) {
                context->VSSetConstantBuffers(WORLD_BUFFER_SLOT, 1, m_worldBuffer.GetAddressOf());
            }

            context->IASetInputLayout(m_layout.Get());
            if (m_linerSampler) context->PSSetSamplers(0, 1, &m_linerSampler);

            // 실행 시점에 멤버에서 참조
            const auto& mat = m_materials[matIndex];

            auto BindTexture = [&](ID3D11ShaderResourceView* srv, UINT slot) {
                if (srv) context->PSSetShaderResources(slot, 1, &srv);
                else {
                    ID3D11ShaderResourceView* nullSRV = nullptr;
                    context->PSSetShaderResources(slot, 1, &nullSRV);
                }
                };

            BindTexture(mat.albedo ? mat.albedo->GetSRV() : nullptr, ABEDO_TEXTURE_SLOT);
            BindTexture(mat.normal ? mat.normal->GetSRV() : nullptr, NORMAL_TEXTURE_SLOT);
            BindTexture(mat.metallic ? mat.metallic->GetSRV() : nullptr, METALLIC_TEXTURE_SLOT);
            BindTexture(mat.ao ? mat.ao->GetSRV() : nullptr, AO_TEXTURE_SLOT);
            BindTexture(mat.roughness ? mat.roughness->GetSRV() : nullptr, SMOOTH_TEXTURE_SLOT);
            BindTexture(mat.emissive ? mat.emissive->GetSRV() : nullptr, EMI_TEXTURE_SLOT);

            currentMesh->RenderBuffer(context);
            };

        params.opaqueQueue->Submit(cmd);
    }
} // Submit

void RigidActor::Animate(float delta) {
    if (m_modelType == AssimpModel::ModelType::RigidAnimated) {
        m_Animator.Update(delta);
    }
} // Animate

DirectX::XMMATRIX RigidActor::GetWorldMatrix() {
    XMMATRIX fixRotation = XMMatrixRotationY(XMConvertToRadians(90.0f));
    return fixRotation * m_transform.GetWorldMatrix();
} // GetWorldMatrix

bool RigidActor::InitShader(ID3D11Device* device, HWND hwnd, const std::wstring& vsPath, const std::wstring& psPath) {
    using namespace ShaderHelper;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    if (!InitVertexShader(device, hwnd, vsPath,
        layoutDesc, ARRAYSIZE(layoutDesc), m_staticVertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(device, hwnd, psPath, m_pixelShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<WorldBuffer>(device, m_worldBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // InitShader

void RigidActor::OnGui() {
    DrawTransformGui();

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.5f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));

    if (ImGui::CollapsingHeader("MODEL INSPECTOR", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PopStyleColor(3); // 조기 팝 주의

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
                ShowStatus("Metallic", mat.hasMetallic);
                ShowStatus("AO", mat.hasAO);
                ShowStatus("Smoothnes", mat.hasSmoothness);

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