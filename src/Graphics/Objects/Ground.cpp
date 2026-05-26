#include "Pch.h"
#include "Ground.h"
#include "Resources/Texture.h"
#include "Components/Frustum.h"
// utils
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"
#include "SharedConstants/BuffersConstants.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
//
#include <DirectXMath.h>
// define
#define QUAD_MAX_LENG      800
#define QUAD_SCALE         15.0f
#define HEIGHT_SCALE       250.0f
#define TEXTURE_SLOT       10
#define SAMPLER_SLOT       5
#define BUFFER_SLOT_WORLD  2
#define BUFFER_SLOT_GROUND 3
#define BUFFER_SLOT_SHADOW 4

using namespace DirectX;
using namespace SharedConstants;
using namespace PathConstants;
using namespace ConstantBuffer;

Ground::Ground() {
    m_quadTree = std::make_unique<QuadTree>();
	m_heightMap = nullptr;
    m_transform = Transform();
	m_prevGoundData.padding1 = -1.0f;
    m_prevShadowData.padding.x = -1.0f;
} // Ground

Ground::~Ground() {
    m_heightMap = nullptr;
} // ~Ground

bool Ground::Init(const InitParams& params) {
    if (params.device == nullptr || params.heightMapTex == nullptr) {
        return false;
    }

    std::vector<QuadTree::TerrainVertex> vertices;
    std::vector<UINT> indices;
    m_heightMap = params.heightMapTex;

    GenerateTerrainGrid(QUAD_MAX_LENG, QUAD_MAX_LENG, QUAD_SCALE, vertices, indices);

    if (!m_quadTree->Init(params.device, vertices, indices)) {
        DebugHelper::DebugPrint("m_quadTree->Init 실패");
        return false;
    }

    if (!InitShader(params.device, params.hwnd)) {
        return false;
    }
    
    m_ShadowData.mapWidth = BuffersConstants::SHADOWMAP_WIDTH;
    m_ShadowData.mapHeight = BuffersConstants::SHADOWMAP_HEIGHT;
    m_ShadowData.spread = BuffersConstants::SPREAD;
    m_ShadowData.bias = BuffersConstants::BIAS;
    return true;
} // Init

void Ground::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    if (!params.frustum) {
        return;
    }

    context->IASetInputLayout(m_layout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    context->PSSetShaderResources(TEXTURE_SLOT, 1, &params.shadowSRV);
    context->PSSetSamplers(SAMPLER_SLOT, 1, &params.shadowSampler);

    XMMATRIX world = XMMatrixIdentity();

    m_worldData.world = XMMatrixTranspose(world);
    if (!ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_worldData)) {
        DebugHelper::DebugPrint("Failed to update world buffer");
        return;
    }
    context->VSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_worldBuffer.GetAddressOf());
    context->PSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_worldBuffer.GetAddressOf());

    if (UpdateGroundBuffer(context)) {
        context->PSSetConstantBuffers(BUFFER_SLOT_GROUND, 1, m_groundBuffer.GetAddressOf());
    }

    if (UpdateShadowBuffer(context, world)) {
        context->PSSetConstantBuffers(BUFFER_SLOT_SHADOW, 1, m_shadowBuffer.GetAddressOf());
    }

    std::vector<QuadTree::QuadTreeNode*> visibleNodes;
    m_quadTree->GetVisibleNodes(params.frustum, visibleNodes);

    UINT stride = sizeof(QuadTree::TerrainVertex);
    UINT offset = 0;

    ID3D11Buffer* globalVB = m_quadTree->GetGlobalVertexBuffer();

    context->IASetVertexBuffers(0, 1, &globalVB, &stride, &offset);

    // 시야에 들어온 노드들만 순회하며 그림
    for (auto* node : visibleNodes) {
        context->IASetIndexBuffer(node->groundIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(node->groundIndexCount, 0, 0);
    }

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(TEXTURE_SLOT, 1, &nullSRV);
} // Render

void Ground::DrawIndexed(ID3D11DeviceContext* context) {

} // DrawIndexed

void Ground::OnGui() {
    ImGui::Begin("Ground Control");

    ImGui::ColorEdit3("Dark Sand", (float*)&m_GoundData.darkSand);
    ImGui::ColorEdit3("Light Sand", (float*)&m_GoundData.lightSand);

    if (ImGui::Button("Reset Colors")) {
        m_GoundData = GroundBuffer();
    }

    ImGui::End();
} // OnGui

XMMATRIX Ground::GetWorldMatrix() {
    return XMMatrixIdentity();
} // GetWorldMatrix

bool Ground::InitShader(ID3D11Device* device, HWND hwnd) {
    using namespace ShaderHelper;
    using namespace ConstantBuffer;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (!InitVertexShader(device, hwnd, PathConstants::GROUND_VS,
        layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(device, hwnd, PathConstants::GROUND_PS, m_pixelShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<WorldBuffer>(device, m_worldBuffer.GetAddressOf()) ||
        !InitConstantBuffer<GroundBuffer>(device, m_groundBuffer.GetAddressOf()) ||
        !InitConstantBuffer<ShadowBuffer>(device, m_shadowBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // InitShader

bool Ground::UpdateGroundBuffer(ID3D11DeviceContext* context) {
    using namespace ShaderHelper;

    if (memcmp(&m_prevGoundData, &m_GoundData, sizeof(GroundBuffer)) == 0) {
        return true;
    }

    if (!UpdateConstantBuffer(context, m_groundBuffer.Get(), m_GoundData)) {
        return false;
    }

    m_prevGoundData = m_GoundData;
    return true;
} // UpdateGroundBuffer

bool Ground::UpdateShadowBuffer(ID3D11DeviceContext* context, const DirectX::XMMATRIX& world) {
    using namespace ShaderHelper;

    m_ShadowData.world = XMMatrixTranspose(world);

    if (memcmp(&m_prevShadowData, &m_ShadowData, sizeof(ShadowBuffer)) == 0) {
        return true;
    }

    if (!UpdateConstantBuffer(context, m_shadowBuffer.Get(), m_ShadowData)) {
        return false;
    }

    m_prevShadowData = m_ShadowData;
    return true;
} // UpdateShadowBuffer

void Ground::GenerateTerrainGrid(int width, int depth, float scale, std::vector<QuadTree::TerrainVertex>& outVertices, std::vector<UINT>& outIndices) {
    outVertices.clear();
    outIndices.clear();
    outVertices.reserve(width * depth);

    float halfWidth = (width * scale) * 0.5f;
    float halfDepth = (depth * scale) * 0.5f;

    int w = m_heightMap->GetWidth();
    int h = m_heightMap->GetHeight();

    float top = m_heightMap->GetPixelHeight(w / 2, 0);        // 위쪽 중앙
    float bottom = m_heightMap->GetPixelHeight(w / 2, h - 1);      // 아래쪽 중앙
    float left = m_heightMap->GetPixelHeight(0, h / 2);      // 왼쪽 중앙
    float right = m_heightMap->GetPixelHeight(w - 1, h / 2);      // 오른쪽 중앙

    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            QuadTree::TerrainVertex v;
            v.position = XMFLOAT3((x * scale) - halfWidth, 0.0f, (z * scale) - halfDepth);

            float sampleU = (float)x / (width - 1);
            float sampleV = (float)z / (depth - 1);

            float innerWidth = (float)(m_heightMap->GetWidth() - 3);  // 안쪽 픽셀 범위
            float innerHeight = (float)(m_heightMap->GetHeight() - 3);

            int texPixelX = 1 + static_cast<int>(sampleU * innerWidth);
            int texPixelY = 1 + static_cast<int>(sampleV * innerHeight);

            float h = m_heightMap->GetPixelHeight(texPixelX, texPixelY);
            v.position.y = h * HEIGHT_SCALE;

            float actualU = (float)x / (width - 1);
            float actualV = (float)z / (depth - 1);
            v.texcoord = XMFLOAT2(actualU, actualV);

            outVertices.push_back(v);
        }
    } // for - vertices

    // 노말계산: 중앙 차분법
    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            int index = (z * width) + x;
            int leftX = max(0, x - 1);
            int rightX = std::min(width - 1, x + 1);
            int bottomZ = max(0, z - 1);
            int topZ = std::min(depth - 1, z + 1);

            float hL = outVertices[(z * width) + leftX].position.y;
            float hR = outVertices[(z * width) + rightX].position.y;
            float hB = outVertices[(bottomZ * width) + x].position.y;
            float hT = outVertices[(topZ * width) + x].position.y;

            float actualDistX = static_cast<float>(rightX - leftX) * scale;
            float actualDistZ = static_cast<float>(topZ - bottomZ) * scale;

            // X축 방향 경사 벡터와 Z축 방향 경사 벡터 구하기
            XMVECTOR tangentX = XMVectorSet(actualDistX, hR - hL, 0.0f, 0.0f);
            XMVECTOR tangentZ = XMVectorSet(0.0f, hT - hB, actualDistZ, 0.0f);
            XMVECTOR normalVec = XMVector3Normalize(XMVector3Cross(tangentZ, tangentX));

            // 계산된 노말을 정점 구조체에 저장
            XMStoreFloat3(&outVertices[index].normal, normalVec);
        }
	} // for - normal

    for (int z = 0; z < depth - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            UINT topLeft = (z * width) + x;
            UINT topRight = topLeft + 1;
            UINT bottomLeft = ((z + 1) * width) + x;
            UINT bottomRight = bottomLeft + 1;

            // 첫 번째 삼각형
            outIndices.push_back(topLeft);
            outIndices.push_back(bottomLeft);
            outIndices.push_back(topRight);

            // 두 번째 삼각형
            outIndices.push_back(bottomLeft);
            outIndices.push_back(bottomRight);
            outIndices.push_back(topRight);
        }
	} // for- indices
} // GenerateTerrainGrid