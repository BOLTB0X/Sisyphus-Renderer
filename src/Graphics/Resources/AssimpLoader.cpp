#include "Pch.h"
#include "AssimpLoader.h"
#include "PBRMesh.h"
#include "Texture.h"
// Components
#include "Components/TextureManager.h"
// Utils
#include "Helpers/DebugHelper.h"
// STL
#include <filesystem>
#include <objbase.h>
#include <DirectXMath.h>
// defines
#define MAX_BONE_INFLUENCE 4

using namespace SharedConstants::PBRTextureConstants;
using namespace DirectX;

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
        aiProcess_ConvertToLeftHanded | aiProcess_LimitBoneWeights);

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        return false;
    }

	ProcessAnimations(scene, outModel);

    std::string directory = std::filesystem::path(path).parent_path().string();

    ProcessMaterials(scene, device, context, directory, outModel);
    ProcessNode(scene->mRootNode, scene, device, context, outModel);

    outModel->m_rootNode = ParseNodeHierarchy(scene->mRootNode);

    return true;
} // LoadMeshModel

void AssimpLoader::ProcessNode(aiNode* node, const aiScene* scene,
    ID3D11Device* device, ID3D11DeviceContext* context, AssimpModel* outModel)
{
    // 현재 노드의 모든 메쉬 처리
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        outModel->AddMesh(ProcessMesh(mesh, scene, device, context, outModel));
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

        outModel->AddMaterial(result.material);
        if (result.commandList) {
            context->ExecuteCommandList(result.commandList.Get(), FALSE);
        }
    }
} // ProcessMaterials

std::unique_ptr<PBRMesh> AssimpLoader::ProcessMesh(aiMesh* mesh,  const aiScene* scene,
    ID3D11Device* device, ID3D11DeviceContext* context, AssimpModel* outModel)  {
    std::vector<PBRMesh::FBRVertex> vertices;
    std::vector<unsigned int> indices;

    vertices.reserve(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        PBRMesh::FBRVertex vertex = {};

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

    if (mesh->HasBones() && outModel->GetModelType() == AssimpModel::ModelType::Skinned) {
        ExtractBoneWeights(vertices, mesh, scene, outModel);
    }

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

void AssimpLoader::ProcessAnimations(const aiScene* scene, AssimpModel* outModel) {
    bool hasAnimation = scene->HasAnimations();
    bool hasBones = false;

    // 씬에 있는 모든 메쉬를 검사해서 하나라도 Bone를 가지고 있는지 확인
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        if (scene->mMeshes[i]->HasBones()) {
            hasBones = true;
            break;
        }
    }

    if (hasAnimation) {
        if (hasBones) {
            outModel->m_modelType = AssimpModel::ModelType::Skinned;
            DebugHelper::DebugPrint("모델 타입: Skinned Animation");
        }
        else {
            outModel->m_modelType = AssimpModel::ModelType::RigidAnimated;
            DebugHelper::DebugPrint("모델 타입: Rigid Animation");
        }
    }
    else {
        outModel->m_modelType = AssimpModel::ModelType::Static;
        DebugHelper::DebugPrint("모델 타입: Static Mesh");
    }

    // 클립 파싱
    for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
        aiAnimation* aiAnim = scene->mAnimations[i];

        AssimpModel::AnimationClip clip;
        clip.name = aiAnim->mName.C_Str();
        clip.duration = static_cast<float>(aiAnim->mDuration);
        clip.ticksPerSecond = static_cast<float>(aiAnim->mTicksPerSecond != 0.0
            ? aiAnim->mTicksPerSecond : 25.0);

        for (unsigned int c = 0; c < aiAnim->mNumChannels; ++c) {
            aiNodeAnim* ch = aiAnim->mChannels[c];

            AssimpModel::AnimationClip::NodeAnim nodeAnim;
            nodeAnim.name = ch->mNodeName.C_Str();

            for (unsigned int k = 0; k < ch->mNumPositionKeys; ++k) {
                nodeAnim.positionKeys.push_back({
                    static_cast<float>(ch->mPositionKeys[k].mTime),
                    { ch->mPositionKeys[k].mValue.x,
                      ch->mPositionKeys[k].mValue.y,
                      ch->mPositionKeys[k].mValue.z }
                    });
            }

            for (unsigned int k = 0; k < ch->mNumRotationKeys; ++k) {
                nodeAnim.rotationKeys.push_back({
                    static_cast<float>(ch->mRotationKeys[k].mTime),
                    { ch->mRotationKeys[k].mValue.x,
                      ch->mRotationKeys[k].mValue.y,
                      ch->mRotationKeys[k].mValue.z,
                      ch->mRotationKeys[k].mValue.w }
                    });
            }

            for (unsigned int k = 0; k < ch->mNumScalingKeys; ++k) {
                nodeAnim.scaleKeys.push_back({
                    static_cast<float>(ch->mScalingKeys[k].mTime),
                    { ch->mScalingKeys[k].mValue.x,
                      ch->mScalingKeys[k].mValue.y,
                      ch->mScalingKeys[k].mValue.z }
                    });
            }

            clip.channels.push_back(std::move(nodeAnim));
        } // for (unsigned int c = 0; c < aiAnim->mNumChannels; ++c)

        outModel->m_clips.push_back(std::move(clip));
    } // for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
} // ProcessAnimations

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

void AssimpLoader::ExtractBoneWeights(std::vector<PBRMesh::FBRVertex>& vertices, aiMesh* mesh, const aiScene* scene, AssimpModel* outModel) {
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        aiBone* aiBone = mesh->mBones[boneIndex];
        std::string boneName = aiBone->mName.C_Str();

        int boneID = -1;
        if (outModel->m_boneInfoMap.find(boneName) == outModel->m_boneInfoMap.end()) {
            // 처음 보는 뼈라면 새 ID 부여
            boneID = outModel->m_boneCounter++;

            AssimpModel::BoneInfo newBoneInfo;
            newBoneInfo.id = boneID;
            newBoneInfo.offsetMatrix = ConvertMatrixToDirectX(aiBone->mOffsetMatrix);

            outModel->m_boneInfoMap[boneName] = newBoneInfo;
        }
        else {
            boneID = outModel->m_boneInfoMap[boneName].id;
        }

        for (unsigned int weightIndex = 0; weightIndex < aiBone->mNumWeights; ++weightIndex) {
            unsigned int vertexID = aiBone->mWeights[weightIndex].mVertexId;
            float weight = aiBone->mWeights[weightIndex].mWeight;

            SetVertexBoneData(vertices[vertexID], boneID, weight);
        }
    }
} // ExtractBoneWeights

void AssimpLoader::SetVertexBoneData(PBRMesh::FBRVertex& vertex, int boneID, float weight) {
    if (weight <= 0.0001f) return;

    unsigned int* boneIDsPtr = reinterpret_cast<unsigned int*>(&vertex.boneIDs);
    float* boneWeightsPtr = reinterpret_cast<float*>(&vertex.boneWeights);

    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if (boneWeightsPtr[i] == 0.0f) {
            boneIDsPtr[i] = boneID;
            boneWeightsPtr[i] = weight;
            return;
        }
    }
} // SetVertexBoneData

DirectX::XMMATRIX AssimpLoader::ConvertMatrixToDirectX(const aiMatrix4x4& from)
{
    DirectX::XMMATRIX to;

    to.r[0] = DirectX::XMVectorSet(from.a1, from.a2, from.a3, from.a4);
    to.r[1] = DirectX::XMVectorSet(from.b1, from.b2, from.b3, from.b4);
    to.r[2] = DirectX::XMVectorSet(from.c1, from.c2, from.c3, from.c4);
    to.r[3] = DirectX::XMVectorSet(from.d1, from.d2, from.d3, from.d4);

    return to;
} // ConvertMatrixToDirectX

std::unique_ptr<AssimpModel::ModelNode> AssimpLoader::ParseNodeHierarchy(aiNode* aiNode)
{
    if (!aiNode) return nullptr;

    auto newNode = std::make_unique<AssimpModel::ModelNode>();
    newNode->name = aiNode->mName.C_Str();

    newNode->transformation = ConvertMatrixToDirectX(aiNode->mTransformation);

    // 자식 노드가 있다면 메모리를 미리 예약하고 재귀적으로 파싱하여 추가
    newNode->children.reserve(aiNode->mNumChildren);
    for (unsigned int i = 0; i < aiNode->mNumChildren; ++i)
    {
        auto childNode = ParseNodeHierarchy(aiNode->mChildren[i]);
        if (childNode)
        {
            newNode->children.push_back(std::move(childNode));
        }
    }

    return newNode;
} // ParseNodeHierarchy