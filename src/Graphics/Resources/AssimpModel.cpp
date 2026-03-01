#include "Pch.h"
#include "AssimpModel.h"
#include "AssimpLoader.h"
#include "Texture.h"
#include "PBRMesh.h"

AssimpModel::AssimpModel() {
    m_AssimpLoader = std::make_unique<AssimpLoader>();
} // AssimpModel

AssimpModel::~AssimpModel() {
} // ~AssimpModel

bool AssimpModel::Init(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path) {
    return m_AssimpLoader->LoadMeshModel(device, context, path, this);
} // Init

void AssimpModel::AddMesh(std::unique_ptr<PBRMesh> newMesh) {
    if (newMesh) m_meshes.push_back(std::move(newMesh));
} // AddMesh

void AssimpModel::AddMaterial(const Material& material) {
    m_materials.push_back(material);
} // AddMaterial