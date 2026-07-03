#include "Pch.h"
#include "InstancingActor.h"
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
#define WORLD_BUFFER_SLOT      2
#define CHECK_LEAF_BUFFER_SLOT 3
#define ABEDO_TEXTURE_SLOT     0
#define NORMAL_TEXTURE_SLOT    1
#define ROUGHNESS_TEXTURE_SLOT 2
#define OPACITY_TEXTURE_SLOT   3
#define SSS_TEXTURE_SLOT       4
#define INS_TEXTURE_SLOT       5

using namespace DirectX;
using namespace SharedConstants;
using namespace ConstantBuffer;

InstancingActor::InstancingActor() : AssimpModel(), ActorObject() {
    m_linerSampler = nullptr;
    m_checkTranspData = CheckTransparentBuffer();
    m_leafKeywords = { "ClusterB", "ClusterB2" };
    m_instanceSRV = nullptr;
    m_instanceUAV = nullptr;
} // InstancedTreeActor 

InstancingActor::~InstancingActor() {
    m_linerSampler = nullptr;
    m_instanceSRV = nullptr;
    m_instanceUAV = nullptr;
} // ~TransparentActor

bool InstancingActor::Init(const InitParams& params) {
    if (params.device == nullptr || params.context == nullptr) {
        return false;
    }

    m_textureMgr = params.textMgr;
    if (!AssimpModel::Init(params.device, params.context, m_textureMgr, params.path)) {
        return false;
    }

    if (!InitShader(params.device, params.hwnd, params.VSPath, params.PSPath)) {
        return false;
    }

    m_linerSampler = params.linerSampler;

    for (size_t i = 0; i < m_meshes.size(); ++i) {
        D3D11_BUFFER_DESC argsDesc = {};
        argsDesc.Usage = D3D11_USAGE_DEFAULT;
        argsDesc.ByteWidth = 20;
        argsDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
        argsDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

        UINT indexCount = m_meshes[i]->GetIndexCount();

        UINT argsInit[5] = { indexCount, 0, 0, 0, 0 };

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = argsInit;

        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        params.device->CreateBuffer(&argsDesc, &initData, buffer.GetAddressOf());
        m_argsBuffers.push_back(buffer);
    }

    return true;
} // Init

void InstancingActor::RenderShadow(ID3D11DeviceContext* context, const RenderShadowParams& params) {
    if (!context || !params.shadowMap || !params.shadowParams || !params.states
        || !params.instanceSRV || !params.instanceUAV) {
        return;
    }

    m_instanceSRV = params.instanceSRV;
    params.shadowParams->isInstanced = true;
    params.shadowParams->worldMatrix = GetWorldMatrix();

    for (size_t i = 0; i < m_meshes.size(); ++i) {
        PBRMesh* currentMesh = m_meshes[i].get();
        ID3D11Buffer* currentArgsBuffer = m_argsBuffers[i].Get();
        unsigned int matIndex = currentMesh->GetMaterialIndex();
        if (matIndex >= m_materials.size()) continue;

        const auto& mat = m_materials[matIndex];
        bool isLeafMesh = IsTransparentMaterial(mat.name);

        params.shadowParams->alphaSRV = mat.alpha ? mat.alpha->GetSRV() : nullptr;

        if (isLeafMesh) {
            context->RSSetState(params.states->GetShadowCullNoneState());
            context->OMSetBlendState(params.states->GetBlendState(), nullptr, 0xFFFFFFFF);
            params.shadowMap->RenderTransparent(context, *params.shadowParams);
        }
        else {
            context->RSSetState(params.states->GetShadowCullBackState());
            context->OMSetBlendState(params.states->GetNoBlendState(), nullptr, 0xFFFFFFFF);
            params.shadowMap->RenderOpaque(context, *params.shadowParams);
        }

        context->VSSetShaderResources(INS_TEXTURE_SLOT, 1, &m_instanceSRV);
        context->CopyStructureCount(currentArgsBuffer, 4, params.instanceUAV);

        currentMesh->RenderInstancedBuffer(context, currentArgsBuffer);

        if (isLeafMesh) {
            ID3D11ShaderResourceView* nullSRV = nullptr;
            context->PSSetShaderResources(0, 1, &nullSRV);
        }
    }

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->VSSetShaderResources(INS_TEXTURE_SLOT, 1, &nullSRV);
} // RenderShadow

void InstancingActor::Submit(const SubmitParams& params) {
    if (!params.opaqueQueue || !params.states || !params.instanceSRV) {
        return;
    }

    m_instanceSRV = params.instanceSRV;
    m_instanceUAV = params.InstanceUAV;

    XMVECTOR posVec = XMLoadFloat3(&m_transform.GetPosition());
    XMVECTOR camVec = XMLoadFloat3(&params.cameraPosition);
    float distance = XMVectorGetX(XMVector3Length(XMVectorSubtract(posVec, camVec)));

    m_worldData.world = XMMatrixTranspose(params.worldMatrix);
    ConstantBuffer::WorldBuffer snapshotWorldData = m_worldData;

    for (size_t i = 0; i < m_meshes.size(); ++i) {
        PBRMesh* currentMesh = m_meshes[i].get();
        ID3D11Buffer* currentArgsBuffer = m_argsBuffers[i].Get();
        unsigned int matIndex = currentMesh->GetMaterialIndex();
        if (matIndex >= m_materials.size()) continue;

        bool isLeaf = IsTransparentMaterial(m_materials[matIndex].name);

        RenderQueue::DrawCommand cmd;
        cmd.vs = m_vertexShader.Get();
        cmd.ps = m_pixelShader.Get();
        cmd.sortKey = GenerateSortKey(params.shaderID, static_cast<uint16_t>(matIndex), distance);

        cmd.execute = [this, currentMesh, matIndex, isLeaf, currentArgsBuffer, states = params.states](ID3D11DeviceContext* context) {

            ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_worldData);
            context->VSSetConstantBuffers(WORLD_BUFFER_SLOT, 1, m_worldBuffer.GetAddressOf());

            CheckTransparentBuffer leafData;
            leafData.isLeaf = isLeaf ? 1 : 0;
            ShaderHelper::UpdateConstantBuffer(context, m_checkLeafBuffer.Get(), leafData);
            context->PSSetConstantBuffers(CHECK_LEAF_BUFFER_SLOT, 1, m_checkLeafBuffer.GetAddressOf());

            // 렌더 상태 변경
            if (isLeaf) {
                context->RSSetState(states->GetCullNone());
                context->OMSetBlendState(states->GetBlendState(), nullptr, 0xFFFFFFFF);
            }
            else {
                context->RSSetState(states->GetCullBackState());
                context->OMSetBlendState(states->GetNoBlendState(), nullptr, 0xFFFFFFFF);
            }

            context->IASetInputLayout(m_layout.Get());
            context->VSSetShaderResources(INS_TEXTURE_SLOT, 1, &m_instanceSRV);
            if (m_linerSampler) {
                context->PSSetSamplers(0, 1, &m_linerSampler);
            }

            auto BindTexture = [&](ID3D11ShaderResourceView* srv, UINT slot) {
                if (srv) {
                    context->PSSetShaderResources(slot, 1, &srv);
                }
                else {
                    ID3D11ShaderResourceView* nullSRV = nullptr;
                    context->PSSetShaderResources(slot, 1, &nullSRV);
                }
                };

            const auto& mat = m_materials[matIndex];
            BindTexture(mat.albedo ? mat.albedo->GetSRV() : nullptr, ABEDO_TEXTURE_SLOT);
            BindTexture(mat.normal ? mat.normal->GetSRV() : nullptr, NORMAL_TEXTURE_SLOT);
            BindTexture(mat.roughness ? mat.roughness->GetSRV() : nullptr, ROUGHNESS_TEXTURE_SLOT);
            BindTexture(mat.alpha ? mat.alpha->GetSRV() : nullptr, OPACITY_TEXTURE_SLOT);
            BindTexture(mat.subsurface ? mat.subsurface->GetSRV() : nullptr, SSS_TEXTURE_SLOT);

            context->CopyStructureCount(currentArgsBuffer, 4, m_instanceUAV);

            currentMesh->RenderInstancedBuffer(context, currentArgsBuffer);

            ID3D11ShaderResourceView* nullSRV = nullptr;
            context->VSSetShaderResources(INS_TEXTURE_SLOT, 1, &nullSRV);
            m_RenderCount++;
        };

        if (isLeaf && params.transparentQueue) {
            params.transparentQueue->Submit(cmd);
        }
        else {
            params.opaqueQueue->Submit(cmd);
        }
    }
} // Submit

void InstancingActor::OnGui() {
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.5f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));

    if (ImGui::CollapsingHeader("MODEL INSPECTOR", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PopStyleColor(3);

        ImGui::Indent();
        ImGui::Spacing();

        ImGui::Text("RenderCount: %u", m_RenderCount);
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

XMMATRIX InstancingActor::GetWorldMatrix() {
    XMMATRIX correction = XMMatrixRotationX(XMConvertToRadians(90.0f));
    return correction * m_transform.GetWorldMatrix();
} // GetWorldMatrix

bool InstancingActor::InitShader(ID3D11Device* device, HWND hwnd, const std::wstring& vsPath, const std::wstring& psPath) {
    using namespace ShaderHelper;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    if (!InitVertexShader(device, hwnd, vsPath,
        layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(device, hwnd, psPath, m_pixelShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<WorldBuffer>(device, m_worldBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<CheckTransparentBuffer>(device, m_checkLeafBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // InitShader

bool InstancingActor::IsTransparentMaterial(const std::string& materialName) const {
    for (const auto& keyword : m_leafKeywords) {
        if (materialName.find(keyword) != std::string::npos) {
            return true;
        }
    }
    return false;
} // IsTransparentMaterial