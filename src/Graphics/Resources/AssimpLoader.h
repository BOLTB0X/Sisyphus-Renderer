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
#include <future>
#include <mutex>
// Resources
#include "AssimpModel.h"
// Utils
#include "SharedConstants/PBRTextureConstants.h"

class Texture;
class TextureManager;

class AssimpLoader {
public:
    AssimpLoader(std::shared_ptr<TextureManager>);
    AssimpLoader(const AssimpLoader& other) = delete;
    ~AssimpLoader();

    bool                      LoadMeshModel(ID3D11Device*, ID3D11DeviceContext*, const std::string&, AssimpModel*);

private:
    void                      ProcessNode(aiNode*, const aiScene*, ID3D11Device*, ID3D11DeviceContext*, AssimpModel*);
    std::unique_ptr<PBRMesh>  ProcessMesh(aiMesh*, const aiScene*, ID3D11Device*, ID3D11DeviceContext*);
    void                      ProcessMaterials(const aiScene*, ID3D11Device*, ID3D11DeviceContext*, const std::string&, AssimpModel*);

    std::shared_ptr<Texture>  LoadMaterialElement(ID3D11Device*, ID3D11DeviceContext*, const std::string&, const std::string&, SharedConstants::PBRTextureConstants::PBRTextureType);

private:
    std::shared_ptr<TextureManager> m_TextureMgr;
    std::mutex m_cacheMutex;
}; // AssimpLoader