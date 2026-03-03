#include "Pch.h"
#include "AssimpModel.h"
#include "AssimpLoader.h"
#include "Texture.h"
#include "TextureManager.h"
#include "PBRMesh.h"

AssimpModel::AssimpModel() {
} // AssimpModel

AssimpModel::~AssimpModel() {
} // ~AssimpModel

bool AssimpModel::Init(ID3D11Device* device, ID3D11DeviceContext* context, std::shared_ptr<TextureManager> texMgr,const std::string& path) {
    m_AssimpLoader = std::make_unique<AssimpLoader>(texMgr);
    return m_AssimpLoader->LoadMeshModel(device, context, path, this);
} // Init

int AssimpModel::GetMeshCount() const {
    return static_cast<int>(m_meshes.size());
} // GetMeshCount

std::vector<AssimpModel::MaterialInfo> AssimpModel::GetMaterialInfos() const {
    std::vector<MaterialInfo> infos;
    for (const auto& mat : m_materials) {
        MaterialInfo info;
        info.name = mat.name;
        info.hasAlbedo = (mat.albedo != nullptr);
        info.hasNormal = (mat.normal != nullptr);
        info.hasMetallic = (mat.metallic != nullptr);
        info.hasRoughness = (mat.roughness != nullptr);
        info.hasAO = (mat.ao != nullptr);
        infos.push_back(info);
    }
    return infos;
} // GetMaterialInfos

void AssimpModel::AddMesh(std::unique_ptr<PBRMesh> newMesh) {
    if (newMesh) m_meshes.push_back(std::move(newMesh));
} // AddMesh

void AssimpModel::AddMaterial(const Material& material) {
    m_materials.push_back(material);
} // AddMaterial