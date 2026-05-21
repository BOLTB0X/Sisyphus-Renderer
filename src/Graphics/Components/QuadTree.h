// QuadTree.h
#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>
#include <vector>

class Frustum;

class QuadTree {
public:
    struct BoxVertex {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT2 texcoord;
    }; // BoxVertex

    struct QuadTreeNode {
        // 트리 생성(분할)에 쓸 데이터
        float centerX, centerZ;
        float width;

        // 컬링 및 범위 검사용
        float boundsMinX, boundsMinZ;
        float boundsMaxX, boundsMaxZ;

        Microsoft::WRL::ComPtr<ID3D11Buffer> groundVertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> groundIndexBuffer;
        int                                  groundIndexCount;

        Microsoft::WRL::ComPtr<ID3D11Buffer> grassSeedVertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> grassSeedIndexBuffer;
        int                                  grassSeedCount;

        std::unique_ptr<QuadTreeNode>        children[4];
        bool                                 isLeaf;

        QuadTreeNode() {
            centerX = 0.0f;
            centerZ = 0.0f;
            width = 0.0f;
            boundsMinX = 0.0f;
            boundsMinZ = 0.0f;
            boundsMaxX = 0.0f;
            boundsMaxZ = 0.0f;
            groundIndexCount = 0;
            grassSeedCount = 0;
			isLeaf = false;
        } // QuadTreeNode

        void CalculateBounds() {
            boundsMinX = centerX - (width / 2.0f);
            boundsMaxX = centerX + (width / 2.0f);
            boundsMinZ = centerZ - (width / 2.0f);
            boundsMaxZ = centerZ + (width / 2.0f);
        } // CalculateBounds

    }; // QuadTreeNode

public:
    QuadTree();
    ~QuadTree();

    bool Init(ID3D11Device*, const std::vector<BoxVertex>&, const std::vector<unsigned long>&);
    void GetVisibleNodes(Frustum*, std::vector<QuadTreeNode*>&);

private:
    void BuildTree(ID3D11Device*, QuadTreeNode*, const std::vector<BoxVertex>&, const std::vector<unsigned long>&);
    void CheckVisibility(QuadTreeNode*, Frustum*, std::vector<QuadTreeNode*>&);

private:
    std::unique_ptr<QuadTreeNode> m_rootNode;
    unsigned int                  m_maxTriangles;
}; // QuadTree