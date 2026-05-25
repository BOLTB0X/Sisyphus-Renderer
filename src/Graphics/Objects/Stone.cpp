#include "Pch.h"
#include "Objects/Stone.h"
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

bool Stone::Init(const InitParams& params) {
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

	m_sampler = params.linerSampler;

    //SetPosition(0.0f, 1.0f, 0.0f);
    return true;
} // Init

void Stone::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    RenderParams shaderParams;
    shaderParams.world      = GetWorldMatrix();

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

void Stone::DrawIndexed(ID3D11DeviceContext* context) {
    for (const auto& mesh : m_meshes) {
        mesh->RenderBuffer(context);

        context->DrawIndexed(mesh->GetIndexCount(), 0, 0);
    }
} // DrawIndexed

void Stone::OnGui() {
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

                ShowStatus("Albedo", mat.hasAlbedo);
                ShowStatus("Normal", mat.hasNormal);
                ShowStatus("Metallic", mat.hasMetallic);
                ShowStatus("Roughness", mat.hasRoughness);
                ShowStatus("AO", mat.hasAO);

                ImGui::EndGroup();
                ImGui::Spacing();
                ImGui::TreePop();
            }
        }

        ImGui::Unindent();
    }
    else {
        ImGui::PopStyleColor(3);
    }
} // OnGui

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

    if (!InitConstantBuffer<WorldBuffer>(device, m_worldBuffer.GetAddressOf())) {
		return false;
    }

    return true;
} // InitShader

bool Stone::RenderShader(ID3D11DeviceContext* context, const RenderParams& params) {
	m_worldData.world = XMMatrixTranspose(params.world);
    if (!ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_worldData)) {
        DebugHelper::DebugPrint("Failed to update world buffer");
        return false;
	}
    context->VSSetConstantBuffers(2, 1, m_worldBuffer.GetAddressOf());
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    if (m_sampler) {
        context->PSSetSamplers(0, 1, &m_sampler);
    }
    return true;
} // RenderShader