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
#include "PBRMesh.h"
// Utils
#include "SharedConstants/PBRTextureConstants.h"

class Texture;
class TextureManager;

class AssimpLoader {
public:
    struct AsyncMatResult {
        AssimpModel::Material                     material;
        Microsoft::WRL::ComPtr<ID3D11CommandList> commandList;
    }; // AsyncMatResult

public:
    AssimpLoader(std::shared_ptr<TextureManager>);
    AssimpLoader(const AssimpLoader& other) = delete;
    ~AssimpLoader();

    bool                      LoadMeshModel(ID3D11Device*, ID3D11DeviceContext*, const std::string&, AssimpModel*);

private:
    void                      ProcessNode(aiNode*, const aiScene*, ID3D11Device*, ID3D11DeviceContext*, AssimpModel*);
    std::unique_ptr<PBRMesh>  ProcessMesh(aiMesh*, const aiScene*, ID3D11Device*, ID3D11DeviceContext*, AssimpModel*);
    void                      ProcessMaterials(const aiScene*, ID3D11Device*, ID3D11DeviceContext*, const std::string&, AssimpModel*);
	void 					  ProcessAnimations(const aiScene*, AssimpModel*);

    std::shared_ptr<Texture>  LoadMaterialElement(ID3D11Device*, ID3D11DeviceContext*, aiMaterial*, const std::string&, const std::string&, const std::string&, SharedConstants::PBRTextureConstants::PBRTextureType);
    void                      ExtractBoneWeights(std::vector<PBRMesh::FBRVertex>&, aiMesh*, const aiScene*, AssimpModel*);
    void                      SetVertexBoneData(PBRMesh::FBRVertex&, int, float);
    DirectX::XMMATRIX         ConvertMatrixToDirectX(const aiMatrix4x4&);

    std::unique_ptr<AssimpModel::ModelNode> ParseNodeHierarchy(aiNode*);

private:
    std::shared_ptr<TextureManager> m_TextureMgr;
    std::mutex                      m_cacheMutex;
}; // AssimpLoader