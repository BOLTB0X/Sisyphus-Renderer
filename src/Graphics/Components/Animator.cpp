#include "Pch.h"
#include "Animator.h"

using namespace DirectX;

void Animator::Init(const AssimpModel* model) {
    m_model = model;
	m_currentClip = nullptr;
	m_currentTime = 0.0f;
    m_boneMatrices.assign(256, XMMatrixIdentity());
} // Init

void Animator::Play(const std::string& clipName) {
    m_currentClip = m_model->GetClip(clipName);
    m_currentTime = 0.0f;
} // Play

void Animator::Update(float deltaTime) {
    if (!m_currentClip || !m_model) {
        return;
    }

    float tps = m_currentClip->ticksPerSecond;
    m_currentTime += deltaTime * tps;
    m_currentTime = fmodf(m_currentTime, m_currentClip->duration);

    UpdateNode(m_model->GetRootNode(), XMMatrixIdentity());
} // Animate

const std::vector<XMMATRIX>& Animator::GetBoneMatrices() const {
    return m_boneMatrices;
} // GetBoneMatrices

const std::unordered_map<std::string, XMMATRIX>& Animator::GetNodeTransforms() const { 
    return m_nodeTransforms; 
} // GetNodeTransforms

void Animator::UpdateNode(const AssimpModel::ModelNode* node, const XMMATRIX& parentTransform) {
    if (!node) {
        return;
    }

    // 노드 기본 트랜스폼
    XMMATRIX localTransform = node->transformation;

    const AssimpModel::AnimationClip::NodeAnim* ch = FindNodeAnim(node->name);
    if (ch) {
        XMMATRIX T = InterpolatePosition(*ch, m_currentTime);
        XMMATRIX R = InterpolateRotation(*ch, m_currentTime);
        XMMATRIX S = InterpolateScale(*ch, m_currentTime);
        localTransform = S * R * T;
    }

    XMMATRIX globalTransform = localTransform * parentTransform;

    const auto& boneMap = m_model->GetBoneInfoMap();
    auto it = boneMap.find(node->name);
    if (it != boneMap.end()) {
        int id = it->second.id;
        m_boneMatrices[id] = it->second.offsetMatrix * globalTransform;
    }

    m_nodeTransforms[node->name] = globalTransform;

    for (const auto& child : node->children) {
        UpdateNode(child.get(), globalTransform);
    }
} // UpdateNode

const AssimpModel::AnimationClip::NodeAnim* Animator::FindNodeAnim(const std::string& nodeName) const {
    for (const auto& ch : m_currentClip->channels)
        if (ch.name == nodeName) {
            return &ch;
        }
    return nullptr;
} // FindNodeAnim

XMMATRIX Animator::InterpolatePosition(const AssimpModel::AnimationClip::NodeAnim& ch, float time) {
    if (ch.positionKeys.size() == 1) {
        return XMMatrixTranslationFromVector(XMLoadFloat3(&ch.positionKeys[0].second));
    }

    for (size_t i = 0; i + 1 < ch.positionKeys.size(); ++i) {
        if (time < ch.positionKeys[i + 1].first) {
            float t = (time - ch.positionKeys[i].first)
                / (ch.positionKeys[i + 1].first - ch.positionKeys[i].first);
            XMVECTOR a = XMLoadFloat3(&ch.positionKeys[i].second);
            XMVECTOR b = XMLoadFloat3(&ch.positionKeys[i + 1].second);
            return XMMatrixTranslationFromVector(XMVectorLerp(a, b, t));
        }
    }
    return XMMatrixTranslationFromVector(XMLoadFloat3(&ch.positionKeys.back().second));
} // InterpolatePosition

XMMATRIX Animator::InterpolateRotation(const AssimpModel::AnimationClip::NodeAnim& ch, float time) {
    if (ch.rotationKeys.size() == 1) {
        return XMMatrixRotationQuaternion(XMLoadFloat4(&ch.rotationKeys[0].second));
    }

    for (size_t i = 0; i + 1 < ch.rotationKeys.size(); ++i) {
        if (time < ch.rotationKeys[i + 1].first) {
            float t = (time - ch.rotationKeys[i].first)
                / (ch.rotationKeys[i + 1].first - ch.rotationKeys[i].first);
            XMVECTOR a = XMLoadFloat4(&ch.rotationKeys[i].second);
            XMVECTOR b = XMLoadFloat4(&ch.rotationKeys[i + 1].second);
            return XMMatrixRotationQuaternion(XMQuaternionSlerp(a, b, t));
        }
    }
    return XMMatrixRotationQuaternion(XMLoadFloat4(&ch.rotationKeys.back().second));
} // InterpolateRotation

XMMATRIX Animator::InterpolateScale(const AssimpModel::AnimationClip::NodeAnim& ch, float time) {
    if (ch.scaleKeys.size() == 1) {
        auto& s = ch.scaleKeys[0].second;
        return XMMatrixScaling(s.x, s.y, s.z);
    }

    for (size_t i = 0; i + 1 < ch.scaleKeys.size(); ++i) {
        if (time < ch.scaleKeys[i + 1].first) {
            float t = (time - ch.scaleKeys[i].first)
                / (ch.scaleKeys[i + 1].first - ch.scaleKeys[i].first);
            XMVECTOR a = XMLoadFloat3(&ch.scaleKeys[i].second);
            XMVECTOR b = XMLoadFloat3(&ch.scaleKeys[i + 1].second);
            XMFLOAT3 s; XMStoreFloat3(&s, XMVectorLerp(a, b, t));
            return XMMatrixScaling(s.x, s.y, s.z);
        }
    }
    auto& s = ch.scaleKeys.back().second;
    return XMMatrixScaling(s.x, s.y, s.z);
} // InterpolateScale