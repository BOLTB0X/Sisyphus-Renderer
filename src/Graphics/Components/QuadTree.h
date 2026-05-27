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
    struct TerrainVertex {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 texcoord;
    }; // TerrainVertex

    struct QuadTreeNode {
        float centerX, centerZ;
        float width;

        // 컬링 및 범위 검사용
        float boundsMinX, boundsMinZ;
        float boundsMaxX, boundsMaxZ;
        float boundsMinY, boundsMaxY;

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
            boundsMinX = -500.0f;
            boundsMinZ = -500.0f;
            boundsMinY = -500.0f;
            boundsMaxY = 500.0f;
            boundsMaxX = 500.0f;
            boundsMaxZ = 500.0f;
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

    bool          Init(ID3D11Device*, const std::vector<TerrainVertex>&, const std::vector<UINT>&);
    static void   CollectAllLeaves(QuadTreeNode*, std::vector<QuadTreeNode*>&);

    void          GetVisibleNodes(Frustum*, std::vector<QuadTreeNode*>&);
    ID3D11Buffer* GetGlobalVertexBuffer() const;
    QuadTreeNode* GetRootNode() const;

private:
    void BuildTree(ID3D11Device*, QuadTreeNode*, const std::vector<TerrainVertex>&, const std::vector<UINT>&);
    void CheckVisibility(QuadTreeNode*, Frustum*, std::vector<QuadTreeNode*>&);

private:
    std::unique_ptr<QuadTreeNode>        m_rootNode;
    UINT                                 m_maxTriangles;
    float                                m_maxHeight;
	float                                m_minHeight;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_globalVertexBuffer;
}; // QuadTree