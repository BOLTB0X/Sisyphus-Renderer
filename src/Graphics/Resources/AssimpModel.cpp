#include "Pch.h"
#include "AssimpModel.h"
#include "AssimpLoader.h"
#include "Texture.h"
#include "PBRMesh.h"
#include "Components/TextureManager.h"

AssimpModel::AssimpModel() {
    m_modelType = ModelType::Static;
    m_boneCounter = 0;
	m_rootNode = nullptr;
} // AssimpModel

AssimpModel::~AssimpModel() {
    m_rootNode = nullptr;
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
		info.hasAlpha = (mat.alpha != nullptr);
		info.hasSpecular = (mat.specular != nullptr);
		info.hasEmissive = (mat.emissive != nullptr);
		info.hasDisplacement = (mat.displacement != nullptr);
		info.hasSubsurface = (mat.subsurface != nullptr);
        infos.push_back(info);
    }
    return infos;
} // GetMaterialInfos

void AssimpModel::AddMesh(std::unique_ptr<PBRMesh> newMesh) {
    if (newMesh) {
        m_meshes.push_back(std::move(newMesh));
    }
} // AddMesh

void AssimpModel::AddMaterial(const Material& material) {
    m_materials.push_back(material);
} // AddMaterial

AssimpModel::ModelType AssimpModel::GetModelType() const {
    return m_modelType;
} // GetModelType

const AssimpModel::ModelNode* AssimpModel::GetRootNode() const {
    return m_rootNode.get();
} // GetRootNode

const AssimpModel::AnimationClip* AssimpModel::GetClip(const std::string& name) const {
    for (const auto& clip : m_clips)
        if (clip.name == name) {
            return &clip;
        }
    return nullptr;
} // GetClip

const std::unordered_map<std::string, AssimpModel::BoneInfo>& AssimpModel::GetBoneInfoMap() const {
    return m_boneInfoMap;
} // GetBoneInfoMap

int AssimpModel::GetBoneCount() const {
    return m_boneCounter;
} // GetBoneCount

const AssimpModel::ModelNode* AssimpModel::GetRootNode() const {
    return m_rootNode.get();
} // GetRootNode