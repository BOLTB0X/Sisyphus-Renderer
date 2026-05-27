// QuadTree.cpp
#include "Pch.h"
#include "QuadTree.h"
#include "Frustum.h"
// Utils
#include "Helpers/DebugHelper.h"
// STL
#include <algorithm>
#include <cfloat>
// define
#define  MAX_TRIANGLES_PER_NODE 10000

using namespace DirectX;

QuadTree::QuadTree()
	: m_maxTriangles(MAX_TRIANGLES_PER_NODE) {
    m_rootNode = std::make_unique<QuadTreeNode>();
    m_maxHeight = -FLT_MAX;
	m_minHeight = FLT_MAX;
} // QuadTree

QuadTree::~QuadTree() {
} // ~QuadTree

bool QuadTree::Init(ID3D11Device* device, const std::vector<TerrainVertex>& vertices, const std::vector<UINT>& indices) {
    if (vertices.empty() || indices.empty()) {
        return false;
    }

    float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
    float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;

    for (const auto& v : vertices) {
        if (v.position.x < minX) minX = v.position.x;
        if (v.position.x > maxX) maxX = v.position.x;
        if (v.position.y < minY) minY = v.position.y;
        if (v.position.y > maxY) maxY = v.position.y;
        if (v.position.z < minZ) minZ = v.position.z;
        if (v.position.z > maxZ) maxZ = v.position.z;
    }

    // 루트 노드의 중심점과 전체 크기 계산
    float rangeX = maxX - minX;
    float rangeZ = maxZ - minZ;
    m_rootNode->centerX = (minX + maxX) / 2.0f;
    m_rootNode->centerZ = (minZ + maxZ) / 2.0f;
    m_rootNode->width = max(rangeX, rangeZ) * 1.01f;

    m_rootNode->CalculateBounds();
    m_rootNode->boundsMinY = minY;
    m_rootNode->boundsMaxY = maxY;

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(TerrainVertex) * vertices.size();
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices.data();
    if (FAILED(device->CreateBuffer(
        &vertexBufferDesc, &vertexData, m_globalVertexBuffer.GetAddressOf()))) {
        return false;
    }

    BuildTree(device, m_rootNode.get(), vertices, indices);

    float realMinY = FLT_MAX, realMaxY = -FLT_MAX;
    for (const auto& v : vertices) {
        realMinY = std::min(realMinY, v.position.y);
        realMaxY = max(realMaxY, v.position.y);
    }
    //DebugHelper::DebugPrint("Real vertex Y: " + std::to_string(realMinY) + " ~ " + std::to_string(realMaxY));
    return true;
} // Init

void QuadTree::CollectAllLeaves(QuadTreeNode* node, std::vector<QuadTreeNode*>& out) {
    if (node == nullptr) {
        return;
    }

    if (node->isLeaf) {
        out.push_back(node);
        return;
    }

    for (int i = 0; i < 4; ++i) {
        CollectAllLeaves(node->children[i].get(), out);
    }
} // CollectAllLeaves

void QuadTree::GetVisibleNodes(Frustum* frustum, std::vector<QuadTreeNode*>& outVisibleNodes) {
    outVisibleNodes.clear();

    if (m_rootNode) {
        CheckVisibility(m_rootNode.get(), frustum, outVisibleNodes);
    }
} // GetVisibleNodes

ID3D11Buffer* QuadTree::GetGlobalVertexBuffer() const { 
    return m_globalVertexBuffer.Get();
} // GetGlobalVertexBuffer

QuadTree::QuadTreeNode* QuadTree::GetRootNode() const {
    return m_rootNode.get();
} // GetRootNode

void QuadTree::BuildTree(ID3D11Device* device, QuadTreeNode* node, const std::vector<TerrainVertex>& vertices, const std::vector<UINT>& indices) {
    int triangleCount = indices.size() / 3;

    // 현재 노드가 담당할 삼각형 개수가 제한치 이하로 떨어졌다면 리프 노드로 확정
    if (triangleCount <= m_maxTriangles) {
        node->isLeaf = true;
        node->groundIndexCount = indices.size();

        node->boundsMinY = FLT_MAX;
        node->boundsMaxY = -FLT_MAX;

        // 현재 노드에 할당된 인덱스에 해당하는 정점들만의 높이를 구함
        for (unsigned long idx : indices) {
            node->boundsMinY = std::min(node->boundsMinY, vertices[idx].position.y);
            node->boundsMaxY = max(node->boundsMaxY, vertices[idx].position.y);
        }

        // Index Buffer 생성
        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        indexBufferDesc.ByteWidth = sizeof(UINT) * indices.size();
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexData = {};
        indexData.pSysMem = indices.data();
        device->CreateBuffer(&indexBufferDesc, &indexData, node->groundIndexBuffer.GetAddressOf());

        // TODO: Grass
        node->grassSeedCount = 0;
        return;
    }

    node->isLeaf = false;
    node->boundsMinY = FLT_MAX;
    node->boundsMaxY = -FLT_MAX;

    for (unsigned long idx : indices) {
        node->boundsMinY = std::min(node->boundsMinY, vertices[idx].position.y);
        node->boundsMaxY = max(node->boundsMaxY, vertices[idx].position.y);
    }

    float childWidth = node->width / 2.0f;
    float offset = childWidth / 2.0f;

    // 자식 노드들의 중심 좌표 정의
    float centers[4][2] = {
        { node->centerX - offset, node->centerZ + offset }, // TopLeft
        { node->centerX + offset, node->centerZ + offset }, // TopRight
        { node->centerX - offset, node->centerZ - offset }, // BottomLeft
		{ node->centerX + offset, node->centerZ - offset }  // BottomRight
    };

    for (int i = 0; i < 4; ++i) {
        node->children[i] = std::make_unique<QuadTreeNode>();
        node->children[i]->centerX = centers[i][0];
        node->children[i]->centerZ = centers[i][1];
        node->children[i]->width = childWidth;
        node->children[i]->CalculateBounds();

        std::vector<UINT> childIndices;

        // 현재 노드가 가진 모든 삼각형을 검사하여 해당 자식 영역에 속하는지 판별
        for (size_t j = 0; j < indices.size(); j += 3) {
            UINT idx1 = indices[j];
            UINT idx2 = indices[j + 1];
            UINT idx3 = indices[j + 2];

            TerrainVertex v1 = vertices[idx1];
            TerrainVertex v2 = vertices[idx2];
            TerrainVertex v3 = vertices[idx3];

            // 삼각형의 중심점을 구함
            float triMinX = std::min(v1.position.x, std::min(v2.position.x, v3.position.x));
            float triMaxX = max(v1.position.x, max(v2.position.x, v3.position.x));
            float triMinZ = std::min(v1.position.z, std::min(v2.position.z, v3.position.z));
            float triMaxZ = max(v1.position.z, max(v2.position.z, v3.position.z));

            if ((triMaxX < node->children[i]->boundsMinX ||
                triMinX > node->children[i]->boundsMaxX ||
                triMaxZ < node->children[i]->boundsMinZ ||
                triMinZ > node->children[i]->boundsMaxZ) == false)
            {
                childIndices.push_back(idx1);
                childIndices.push_back(idx2);
                childIndices.push_back(idx3);
            }
        } // for (size_t j = 0; j < indices.size(); j += 3)

        // 해당 영역에 할당된 삼각형이 있다면 재귀적으로 트리를 더 타고 진행
        if (!childIndices.empty()) {
            BuildTree(device, node->children[i].get(), vertices, childIndices);
        }
        else {
            // 해당 영역이 텅 비어있다면 불필요한 노드를 메모리에서 삭제
            node->children[i].reset();
        }
    } // for (int i = 0; i < 4; ++i)

    node->boundsMinY = FLT_MAX;
    node->boundsMaxY = -FLT_MAX;

    for (int i = 0; i < 4; ++i) {
        if (node->children[i]) {
            node->boundsMinY = std::min(node->boundsMinY, node->children[i]->boundsMinY);
            node->boundsMaxY = max(node->boundsMaxY, node->children[i]->boundsMaxY);
        }
    }
} // BuildTree

void QuadTree::CheckVisibility(QuadTreeNode* node, Frustum* frustum, std::vector<QuadTreeNode*>& outVisibleNodes) {
    if (node == nullptr) {
        return;
    }

    bool isInside = frustum->CheckBoundingBoxMinMax(
        node->boundsMaxX, node->boundsMaxY, node->boundsMaxZ,
        node->boundsMinX, node->boundsMinY, node->boundsMinZ
    );
    // 시야에서 벗어났다면 이 노드 아래로는 검사할 필요도 없이 전부 컬링
    if (isInside == false) {
        return;
    }

    if (node->isLeaf) { // 시야에 들어왔고 리프 노드라면 렌더링
        outVisibleNodes.push_back(node);
    }
    else { // 내부 노드라면 자식 노드들로 더 세밀하게 파고들어 검사
        for (int i = 0; i < 4; ++i) {
            if (node->children[i]) {
                CheckVisibility(node->children[i].get(), frustum, outVisibleNodes);
            }
        }
    } // if - else

} // CheckVisibility