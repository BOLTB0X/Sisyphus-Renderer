#pragma once
#include <d3d11.h>
// STL
#include <vector>
#include <string>
#include <memory>
#include "VertexTypes.h"

class AssimpLoader;
class Texture;
class TextureManager;
class PBRMesh;

class AssimpModel {
    friend class AssimpLoader;
public:
    struct MaterialInfo {
        std::string name;
        bool hasAlbedo;
        bool hasNormal;
		bool hasMetallic;
		bool hasRoughness;
		bool hasAO;

        MaterialInfo() {
            name = "";
            hasAlbedo = false;
			hasNormal = false;
			hasMetallic = false;
			hasRoughness = false;
			hasAO = false;
        }
    };

    AssimpModel();
    virtual ~AssimpModel();

	int                       GetMeshCount() const;
    std::vector<MaterialInfo> GetMaterialInfos() const;
    
protected:
    struct Material {
        std::string name;
        std::shared_ptr<Texture> albedo;
        std::shared_ptr<Texture> normal;
        std::shared_ptr<Texture> metallic;
        std::shared_ptr<Texture> roughness;
        std::shared_ptr<Texture> ao;
    };
    bool Init(ID3D11Device*, ID3D11DeviceContext*, std::shared_ptr<TextureManager>, const std::string&);
    void AddMesh(std::unique_ptr<PBRMesh>);
    void AddMaterial(const Material&);

protected:
    std::unique_ptr<AssimpLoader>         m_AssimpLoader;
    std::shared_ptr<TextureManager>       m_TextureMgr;
    std::vector<std::unique_ptr<PBRMesh>> m_meshes;
    std::vector<Material>                 m_materials;
}; // AssimpModel