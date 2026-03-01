#pragma once
#include <d3d11.h>
// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
// STL
#include <string>
#include <memory>
#include <vector>
// 
#include "AssimpModel.h"
// Utils
#include "SharedConstants/PBRTextureConstants.h"

class Texture;

class AssimpLoader {
public:
    AssimpLoader();
    AssimpLoader(const AssimpLoader& other) = delete;
    ~AssimpLoader();

    bool                      LoadMeshModel(ID3D11Device*, ID3D11DeviceContext*, const std::string&, AssimpModel*);

private:
    void                      ProcessNode(aiNode*, const aiScene*, ID3D11Device*, ID3D11DeviceContext*, AssimpModel*);
    std::unique_ptr<PBRMesh>  ProcessMesh(aiMesh*, const aiScene*, ID3D11Device*, ID3D11DeviceContext*);
    void                      ProcessMaterials(const aiScene*, ID3D11Device*, ID3D11DeviceContext*, const std::string&, AssimpModel*);

    std::shared_ptr<Texture>  LoadMaterialElement(ID3D11Device*, ID3D11DeviceContext*, const std::string&, const std::string&, SharedConstants::PBRTextureConstants::PBRTextureType);
}; // AssimpLoader