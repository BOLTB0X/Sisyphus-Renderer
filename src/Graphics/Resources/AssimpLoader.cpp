#include "Pch.h"
#include "AssimpLoader.h"
#include "PBRMesh.h"
#include "Texture.h"
// STL
#include <filesystem>

using namespace SharedConstants::PBRTextureConstants;

AssimpLoader::AssimpLoader() {}
AssimpLoader::~AssimpLoader() {}

bool AssimpLoader::LoadMeshModel(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path, AssimpModel* outModel) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices |
        aiProcess_ConvertToLeftHanded);

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        return false;
    }

    std::string directory = std::filesystem::path(path).parent_path().string();

    ProcessMaterials(scene, device, context, directory, outModel);
    ProcessNode(scene->mRootNode, scene, device, context, outModel);

    return true;
} // LoadMeshModel

void AssimpLoader::ProcessNode(aiNode* node, const aiScene* scene,
    ID3D11Device* device, ID3D11DeviceContext* context, AssimpModel* outModel)
{
    // 현재 노드의 모든 메쉬 처리
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        outModel->AddMesh(ProcessMesh(mesh, scene, device, context));
    }

    // 자식 노드들도 처리
    for (unsigned int i = 0; i < node->mNumChildren; i++)
        ProcessNode(node->mChildren[i], scene, device, context, outModel);

    return;
} // ProcessNode

void AssimpLoader::ProcessMaterials(const aiScene* scene, ID3D11Device* device, ID3D11DeviceContext* context,
    const std::string& directory, AssimpModel* outModel) {
    std::string pbrDir = directory + "/textures/";
    if (!std::filesystem::exists(pbrDir)) pbrDir = directory + "/";

    std::string modelName = std::filesystem::path(directory).stem().string();

    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* aiMat = scene->mMaterials[i];
        AssimpModel::Material myMaterial;
        myMaterial.name = aiMat->GetName().C_Str();

        myMaterial.albedo    = LoadMaterialElement(device, context, pbrDir, modelName, PBRTextureType::Albedo);
        myMaterial.normal    = LoadMaterialElement(device, context, pbrDir, modelName, PBRTextureType::Normal);
        myMaterial.metallic  = LoadMaterialElement(device, context, pbrDir, modelName, PBRTextureType::Metallic);
        myMaterial.roughness = LoadMaterialElement(device, context, pbrDir, modelName, PBRTextureType::Roughness);
        myMaterial.ao        = LoadMaterialElement(device, context, pbrDir, modelName, PBRTextureType::AO);

        outModel->AddMaterial(myMaterial);
    }
} // ProcessMaterials

std::unique_ptr<PBRMesh> AssimpLoader::ProcessMesh(aiMesh* mesh, 
    const aiScene* scene, ID3D11Device* device, ID3D11DeviceContext* context)  {
    std::vector<VertexTypes::FBRVertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        VertexTypes::FBRVertex vertex;
        
        vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

        if (mesh->mTextureCoords[0])
            vertex.texture = { (float)mesh->mTextureCoords[0][i].x, (float)mesh->mTextureCoords[0][i].y };
        else
            vertex.texture = { 0.0f, 0.0f };

        if (mesh->HasNormals())
            vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        
        if (mesh->HasTangentsAndBitangents()) {
            vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
            vertex.binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
        }

        vertices.push_back(vertex);
    } // for (unsigned int i = 0; i < mesh->mNumVertices; i++)

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    auto newMesh = std::make_unique<PBRMesh>();
    if (!newMesh->Init(device, vertices, indices, mesh->mMaterialIndex)) {
        return nullptr;
    }
    
    return newMesh;
} // ProcessMesh

std::shared_ptr<Texture> AssimpLoader::LoadMaterialElement(ID3D11Device* device, ID3D11DeviceContext* context,
    const std::string& pbrDir, const std::string& modelName, PBRTextureType type) {
    
    const std::vector<std::string>* keywords = nullptr;
    for (const auto& entry : PBRTEXTURE_KEYWORD_MAP) {
        if (entry.type == type) { 
            keywords = &entry.keywords;
            break; 
        }
    }
    if (!keywords) return nullptr;

    static const std::vector<std::string> exts = { ".png", ".PNG", ".jpg", ".jpeg", ".JPG", ".JPEG", ".tga", ".dds"};

    for (const auto& key : *keywords) {
        for (const auto& ext : exts) {
            std::string fullPath = pbrDir + modelName + key + ext;
            if (std::filesystem::exists(fullPath)) {
                auto texture = std::make_shared<Texture>();
                
                if (texture->Init(device, context, fullPath)) {
                    return texture;
                }
            }
        }
    }
    return nullptr;
} // LoadMaterialElement