#include "Pch.h"
#include "AssimpLoader.h"
#include "PBRMesh.h"
#include "Texture.h"
#include "Components/TextureManager.h"
// Utils
#include "Helpers/DebugHelper.h"
// STL
#include <filesystem>
#include <objbase.h>

using namespace SharedConstants::PBRTextureConstants;

AssimpLoader::AssimpLoader(std::shared_ptr<TextureManager> texMgr)
    : m_TextureMgr(texMgr) {
} // AssimpLoader

AssimpLoader::~AssimpLoader() {
} // ~AssimpLoader

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
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene, device, context, outModel);
    }

    return;
} // ProcessNode

void AssimpLoader::ProcessMaterials(const aiScene* scene, ID3D11Device* device, ID3D11DeviceContext* context,
    const std::string& directory, AssimpModel* outModel) {
    std::string pbrDir = (std::filesystem::exists(directory + "/textures/")) ? directory + "/textures/" : directory + "/";
    std::string modelName = std::filesystem::path(directory).stem().string();

    std::vector<std::future<AsyncMatResult>> futures;

    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* aiMat = scene->mMaterials[i];
        std::string matName = aiMat->GetName().C_Str();

        futures.push_back(std::async(std::launch::async, [=]() {
            HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

            Microsoft::WRL::ComPtr<ID3D11DeviceContext> deferredContext;
            device->CreateDeferredContext(0, deferredContext.GetAddressOf());

            AssimpModel::Material myMaterial;
            myMaterial.name = aiMat->GetName().C_Str();

            // 지연 컨텍스트(deferredContext)를 사용하여 안전하게 텍스처 로드
            myMaterial.albedo = LoadMaterialElement(device, deferredContext.Get(), aiMat, pbrDir, modelName, myMaterial.name, PBRTextureType::Albedo);
            myMaterial.normal = LoadMaterialElement(device, deferredContext.Get(), aiMat, pbrDir, modelName, myMaterial.name, PBRTextureType::Normal);
            myMaterial.metallic = LoadMaterialElement(device, deferredContext.Get(), aiMat, pbrDir, modelName, myMaterial.name, PBRTextureType::Metallic);
            myMaterial.roughness = LoadMaterialElement(device, deferredContext.Get(), aiMat, pbrDir, modelName, myMaterial.name, PBRTextureType::Roughness);
            myMaterial.ao = LoadMaterialElement(device, deferredContext.Get(), aiMat, pbrDir, modelName, myMaterial.name, PBRTextureType::AO);
            myMaterial.alpha = LoadMaterialElement(device, deferredContext.Get(), aiMat, pbrDir, modelName, myMaterial.name, PBRTextureType::Alpha);
            myMaterial.specular = LoadMaterialElement(device, deferredContext.Get(), aiMat, pbrDir, modelName, myMaterial.name, PBRTextureType::Specular);
            myMaterial.emissive = LoadMaterialElement(device, deferredContext.Get(), aiMat, pbrDir, modelName, myMaterial.name, PBRTextureType::Emissive);
            myMaterial.displacement = LoadMaterialElement(device, deferredContext.Get(), aiMat, pbrDir, modelName, myMaterial.name, PBRTextureType::Displacement);
            myMaterial.subsurface = LoadMaterialElement(device, deferredContext.Get(), aiMat, pbrDir, modelName, myMaterial.name, PBRTextureType::Subsurface);

            Microsoft::WRL::ComPtr<ID3D11CommandList> commandList;
            deferredContext->FinishCommandList(FALSE, commandList.GetAddressOf());

            if (SUCCEEDED(hrCo)) {
                CoUninitialize();
            }

            // 로드된 머테리얼 데이터와 커맨드 리스트를 구조체로 묶어서 반환
            AsyncMatResult result;
            result.material = myMaterial;
            result.commandList = commandList;

            return result;
            })); // futures
    } // for - material


    for (auto& f : futures) {
        AsyncMatResult result = f.get(); // 해당 인덱스 스레드가 끝날 때까지 동기 대기

        // 메인 스레드에서 순서대로 집어넣으므로 뮤텍스 락도 필요 없어짐
        outModel->AddMaterial(result.material);
        if (result.commandList) {
            context->ExecuteCommandList(result.commandList.Get(), FALSE);
        }
    }
} // ProcessMaterials

std::unique_ptr<PBRMesh> AssimpLoader::ProcessMesh(aiMesh* mesh,  const aiScene* scene,
    ID3D11Device* device, ID3D11DeviceContext* context)  {
    std::vector<PBRMesh::FBRVertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        PBRMesh::FBRVertex vertex;
        
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
    } // for - i

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
    aiMaterial* aiMat, const std::string& pbrDir, const std::string& modelName, const std::string& matName, PBRTextureType type) {

    // FBX 파일 내부에 텍스처 경로가 저장되어 있는지 확인
    aiTextureType aiType = aiTextureType_NONE;
    if (type == PBRTextureType::Albedo) aiType = aiTextureType_DIFFUSE;
    else if (type == PBRTextureType::Normal) aiType = aiTextureType_NORMALS;
    else if (type == PBRTextureType::Metallic) aiType = aiTextureType_METALNESS;
    else if (type == PBRTextureType::Roughness) aiType = aiTextureType_DIFFUSE_ROUGHNESS;
    else if (type == PBRTextureType::Alpha) aiType = aiTextureType_OPACITY;
    else if (type == PBRTextureType::AO) aiType = aiTextureType_LIGHTMAP;
    else if (type == PBRTextureType::Emissive) aiType = aiTextureType_EMISSIVE;
    else if (type == PBRTextureType::Displacement) aiType = aiTextureType_DISPLACEMENT;
	else if (type == PBRTextureType::Specular) aiType = aiTextureType_SPECULAR;
	else if (type == PBRTextureType::Subsurface) aiType = aiTextureType_NONE;

    if (aiType != aiTextureType_NONE && aiMat->GetTextureCount(aiType) > 0) {
        aiString str;
        aiMat->GetTexture(aiType, 0, &str);
        std::string texPath = pbrDir + str.C_Str();
    
        if (std::filesystem::exists(texPath)) {
            auto tex = m_TextureMgr->GetTexture(device, context, texPath);
            if (tex) return tex;
        }
    }

    // FBX에 경로가 없거나 끊어졌다면 기존의 "키워드" 검색
    const std::vector<std::string>* keywords = nullptr;
    for (const auto& entry : PBRTEXTURE_KEYWORD_MAP) {
        if (entry.type == type) {
            keywords = &entry.keywords;
            break;
        }
    }
    if (!keywords) {
        return nullptr;
    }

    static const std::vector<std::string> exts = { ".png", ".jpg", ".jpeg", ".tga", ".dds" };

    std::vector<std::string> prefixes = { matName, modelName };

    for (const auto& prefix : prefixes) {
        for (const auto& key : *keywords) {
            for (const auto& ext : exts) {
                std::string fullPath = pbrDir + prefix + key + ext;
                if (std::filesystem::exists(fullPath)) {
                    auto tex = m_TextureMgr->GetTexture(device, context, fullPath);
                    if (tex) {
                        //DebugHelper::DebugPrint("Loaded Texture: " + fullPath + " for Material: " + matName);
                        return tex;
                    }
                }
            } // for - ext
        } // for - key
    } // for - prefix

    return nullptr;
} // LoadMaterialElement
