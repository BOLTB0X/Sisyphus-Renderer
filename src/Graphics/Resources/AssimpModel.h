#pragma once
#include <d3d11.h>
// STL
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

class AssimpLoader;
class Texture;
class TextureManager;
class PBRMesh;

class AssimpModel {
    friend class AssimpLoader; friend class Animator;
public:
    struct MaterialInfo {
        std::string name;
        bool hasAlbedo;
        bool hasNormal;
		bool hasMetallic;
		bool hasRoughness;
		bool hasAO;
		bool hasAlpha;
		bool hasSpecular;
		bool hasEmissive;
		bool hasDisplacement;
		bool hasSubsurface;

        MaterialInfo() {
            name = "";
            hasAlbedo = false;
			hasNormal = false;
			hasMetallic = false;
			hasRoughness = false;
			hasAO = false;
			hasAlpha = false;
			hasSpecular = false;
			hasEmissive = false;
			hasDisplacement = false;
			hasSubsurface = false;
        }
    }; // MaterialInfo

    struct BoneInfo {
        int               id;
        DirectX::XMMATRIX offsetMatrix;
    }; // BoneInfo

public:
    AssimpModel();
    virtual ~AssimpModel();

	int                       GetMeshCount() const;
    std::vector<MaterialInfo> GetMaterialInfos() const;

protected:
    enum class ModelType {
        Static,
        RigidAnimated,
        Skinned
    }; // ModelType

    struct ModelNode {
        std::string                             name;
        DirectX::XMMATRIX                       transformation;
        std::vector<std::unique_ptr<ModelNode>> children;

        ModelNode() : name(""), transformation(DirectX::XMMatrixIdentity()) {
            children = std::vector<std::unique_ptr<ModelNode>>();
        } // ModelNode
    }; // ModelNode

    struct Material {
        std::string              name;
        std::shared_ptr<Texture> albedo;
        std::shared_ptr<Texture> normal;
        std::shared_ptr<Texture> metallic;
        std::shared_ptr<Texture> roughness;
        std::shared_ptr<Texture> ao;
        std::shared_ptr<Texture> alpha;
        std::shared_ptr<Texture> specular;
        std::shared_ptr<Texture> emissive;
		std::shared_ptr<Texture> displacement;
        std::shared_ptr<Texture> subsurface;

    }; // Material

    struct AnimationClip {
        std::string name;
        float       duration;
        float       ticksPerSecond;

        struct NodeAnim {
            std::string                                         name;
            std::vector<std::pair<float, DirectX::XMFLOAT3>>    positionKeys;
            std::vector<std::pair<float, DirectX::XMFLOAT4>>    rotationKeys;
            std::vector<std::pair<float, DirectX::XMFLOAT3>>    scaleKeys;
        };
        std::vector<NodeAnim> channels;
    }; // AnimationClip

protected:
    bool Init(ID3D11Device*, ID3D11DeviceContext*, std::shared_ptr<TextureManager>, const std::string&);
    void AddMesh(std::unique_ptr<PBRMesh>);
    void AddMaterial(const Material&);

    ModelType                 GetModelType() const;
    const ModelNode*          GetRootNode() const;
    const AnimationClip*      GetClip(const std::string& name) const;

    const std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() const;
    int                                              GetBoneCount()   const;
    const ModelNode*                                 GetRootNode()    const;

protected:
    std::unique_ptr<AssimpLoader>             m_AssimpLoader;
    std::shared_ptr<TextureManager>           m_TextureMgr;
    std::vector<std::unique_ptr<PBRMesh>>     m_meshes;
    std::vector<Material>                     m_materials;
    ModelType                                 m_modelType;
    std::unique_ptr<ModelNode>                m_rootNode;
    std::unordered_map<std::string, BoneInfo> m_boneInfoMap;
    int                                       m_boneCounter;
    std::vector<AnimationClip>                m_clips;
}; // AssimpModel