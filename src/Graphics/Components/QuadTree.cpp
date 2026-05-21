// QuadTree.cpp
#include "Pch.h"
#include "QuadTree.h"
#include "Frustum.h"
#include "Helpers/DebugHelper.h"
#include <algorithm>
#define  MAX_TRIANGLES_PER_NODE 10000
#define  MAX_POS 999999.0f
#define  MIN_POS -999999.0f

using namespace DirectX;

QuadTree::QuadTree()
	: m_maxTriangles(MAX_TRIANGLES_PER_NODE) {
    m_rootNode = std::make_unique<QuadTreeNode>();
} // QuadTree

QuadTree::~QuadTree() {
} // ~QuadTree

bool QuadTree::Init(ID3D11Device* device, const std::vector<BoxVertex>& vertices, const std::vector<unsigned long>& indices) {
    if (vertices.empty() || indices.empty()) {
        return false;
    }

    float minX = MAX_POS, minZ = MAX_POS;
    float maxX = MIN_POS, maxZ = MIN_POS;

    for (const auto& v : vertices) {
        if (v.position.x < minX) minX = v.position.x;
        if (v.position.x > maxX) maxX = v.position.x;
        if (v.position.z < minZ) minZ = v.position.z;
        if (v.position.z > maxZ) maxZ = v.position.z;
    }

    // 루트 노드의 중심점과 전체 크기 계산
    m_rootNode->centerX = (minX + maxX) / 2.0f;
    m_rootNode->centerZ = (minZ + maxZ) / 2.0f;
    m_rootNode->width = max(maxX - minX, maxZ - minZ);

    m_rootNode->CalculateBounds();

    BuildTree(device, m_rootNode.get(), vertices, indices);

    return true;
} // Init

void QuadTree::GetVisibleNodes(Frustum* frustum, std::vector<QuadTreeNode*>& outVisibleNodes) {
    outVisibleNodes.clear();

    if (m_rootNode) {
        CheckVisibility(m_rootNode.get(), frustum, outVisibleNodes);
    }
} // GetVisibleNodes

void QuadTree::BuildTree(ID3D11Device* device, QuadTreeNode* node, const std::vector<BoxVertex>& vertices, const std::vector<unsigned long>& indices) {
    int triangleCount = indices.size() / 3;

    // 현재 노드가 담당할 삼각형 개수가 제한치 이하로 떨어졌다면 리프 노드로 확정
    if (triangleCount <= m_maxTriangles) {
        node->isLeaf = true;
        node->groundIndexCount = indices.size();
        DebugHelper::DebugPrint("Leaf index count: " + std::to_string(indices.size()));

        // Vertex Buffer 생성
        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexBufferDesc.ByteWidth = sizeof(BoxVertex) * vertices.size();
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexData = {};
        vertexData.pSysMem = vertices.data();
        device->CreateBuffer(&vertexBufferDesc, &vertexData, node->groundVertexBuffer.GetAddressOf());

        // Index Buffer 생성
        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        indexBufferDesc.ByteWidth = sizeof(unsigned long) * indices.size();
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexData = {};
        indexData.pSysMem = indices.data();
        device->CreateBuffer(&indexBufferDesc, &indexData, node->groundIndexBuffer.GetAddressOf());

        // TODO: Grass
        node->grassSeedCount = 0;
        return;
    }

    node->isLeaf = false;

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

        std::vector<BoxVertex> childVertices;
        std::vector<unsigned long> childIndices;

        // 현재 노드가 가진 모든 삼각형을 검사하여 해당 자식 영역에 속하는지 판별
        for (size_t j = 0; j < indices.size(); j += 3) {
            BoxVertex v1 = vertices[indices[j]];
            BoxVertex v2 = vertices[indices[j + 1]];
            BoxVertex v3 = vertices[indices[j + 2]];

            // 삼각형의 중심점을 구함
            float triCenterX = (v1.position.x + v2.position.x + v3.position.x) / 3.0f;
            float triCenterZ = (v1.position.z + v2.position.z + v3.position.z) / 3.0f;

            // 삼각형 중심이 자식 노드의 Bounding Box 안에 있는지 확인
            if (triCenterX >= node->children[i]->boundsMinX && triCenterX <= node->children[i]->boundsMaxX &&
                triCenterZ >= node->children[i]->boundsMinZ && triCenterZ <= node->children[i]->boundsMaxZ)
            {
                childVertices.push_back(v1);
                childVertices.push_back(v2);
                childVertices.push_back(v3);

                unsigned long currentIndex = static_cast<unsigned long>(childIndices.size());
                childIndices.push_back(currentIndex);
                childIndices.push_back(currentIndex + 1);
                childIndices.push_back(currentIndex + 2);
            }
        }

        // 해당 영역에 할당된 삼각형이 있다면 재귀적으로 트리를 더 타고 진행
        if (!childIndices.empty()) {
            BuildTree(device, node->children[i].get(), childVertices, childIndices);
        }
        else {
            // 해당 영역이 텅 비어있다면 불필요한 노드를 메모리에서 삭제
            node->children[i].reset();
        }
    }
} // BuildTree

void QuadTree::CheckVisibility(QuadTreeNode* node, Frustum* frustum, std::vector<QuadTreeNode*>& outVisibleNodes) {
    if (!node) return;

    // Y축(높이)은 지형의 최대/최소 높이 범위로 넉넉하게 설정
    float minHeight = -500.0f;
    float maxHeight = 500.0f;

    // 절두체 Bounding Box 교차 검사
    bool isInside = frustum->CheckBoundingBoxMinMax(
        node->boundsMaxX, maxHeight, node->boundsMaxZ,
        node->boundsMinX, minHeight, node->boundsMinZ
    );

    // 시야에서 벗어났다면 이 노드 아래로는 검사할 필요도 없이 전부 컬링
    if (!isInside) {
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