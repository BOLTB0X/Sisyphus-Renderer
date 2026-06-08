#pragma once
#include "Resources/AssimpModel.h"
// STL
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <unordered_map>

class Animator {
public:
    void  Init(const AssimpModel*);
    void  Play(const std::string&);
    void  Update(float);

    const std::vector<DirectX::XMMATRIX>& GetBoneMatrices() const;

private:
    void UpdateNode(const AssimpModel::ModelNode*, const DirectX::XMMATRIX&);

    const AssimpModel::AnimationClip::NodeAnim* FindNodeAnim(const std::string&) const;

    DirectX::XMMATRIX InterpolatePosition(const AssimpModel::AnimationClip::NodeAnim&, float);
    DirectX::XMMATRIX InterpolateRotation(const AssimpModel::AnimationClip::NodeAnim&, float);
    DirectX::XMMATRIX InterpolateScale(const AssimpModel::AnimationClip::NodeAnim&, float);

private:
    const AssimpModel*                m_model;
    const AssimpModel::AnimationClip* m_currentClip;
    float                             m_currentTime;
    std::vector<DirectX::XMMATRIX>    m_finalBoneMatrices;
}; // Animator