#include "Pch.h"
#include "Terrain.h"
#include "Resources/Texture.h"
#include "Components/Frustum.h"
#include "D3D11/D3D11State.h"
// utils
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"
#include "SharedConstants/BuffersConstants.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
#include "Helpers/MathHelper.h"

#define TEX_HEIGHTMAP_SLOT       0
#define TEX_NOR_SLOT             2
#define TEX_SAND_SLOT            3
#define TEX_GRASS_SLOT           4
#define TEX_DIFF_SLOT            5
#define TEX_SNOW_SLOT            6
#define LINEAR_SAMPLER_SLOT      0
#define BUFFER_SLOT_LIGHT_MATRIX 1
#define BUFFER_SLOT_WORLD        2
#define BUFFER_SLOT_TESS         3
#define BUFFER_SLOT_HEIGHT       4
#define BUFFER_SLOT_BLEND        5

using namespace DirectX;
using namespace SharedConstants;
using namespace ConstantBuffer;

Terrain::Terrain() {
    m_indexCount = 0;
    m_patchCountX = 32;
    m_patchCountZ = 32;
    m_patchSize = 100.0f; // 패치 1개의 크기
    m_norSRV = nullptr;
    m_sandSRV = nullptr;
    m_grassSRV = nullptr;
    m_snowSRV = nullptr;
    m_diffSRV = nullptr;
    m_linearSampler = nullptr;
} // Terrain

Terrain::~Terrain() {
    m_norSRV = nullptr;
    m_sandSRV = nullptr;
    m_grassSRV = nullptr;
    m_snowSRV = nullptr;
    m_diffSRV = nullptr;
    m_linearSampler = nullptr;
} // ~Terrain

bool Terrain::Init(const InitParams& params) {
    if (params.device == nullptr || params.heightMapTex == nullptr) return false;

    m_heightMap = params.heightMapTex;
    m_norSRV = params.norSRV;
    m_sandSRV = params.sandSRV;
    m_grassSRV = params.grassSRV;
    m_snowSRV = params.snowSRV;
    m_diffSRV = params.diffSRV;
    m_linearSampler = params.linearSampler;

    std::vector<TerrainVertex> vertices;
    std::vector<UINT> indices;

    GeneratePatchGrid(m_patchCountX, m_patchCountZ, m_patchSize, vertices, indices);

    // 버텍스 버퍼 생성
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(TerrainVertex) * vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vinitData = {};
    vinitData.pSysMem = vertices.data();
    params.device->CreateBuffer(&vbd, &vinitData, m_vertexBuffer.GetAddressOf());

    // 인덱스 버퍼 생성
    m_indexCount = indices.size();
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(UINT) * m_indexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iinitData = {};
    iinitData.pSysMem = indices.data();
    params.device->CreateBuffer(&ibd, &iinitData, m_indexBuffer.GetAddressOf());

    if (!InitShader(params.device, params.hwnd)) {
        return false;
    }
    return true;
} // Init

void Terrain::GeneratePatchGrid(int patchCountX, int patchCountZ, float patchSize,
    std::vector<TerrainVertex>& outVertices, std::vector<UINT>& outIndices) {
    outVertices.clear();
    outIndices.clear();

    int vertexCountX = patchCountX + 1;
    int vertexCountZ = patchCountZ + 1;
    float halfWidth = (patchCountX * patchSize) * 0.5f;
    float halfDepth = (patchCountZ * patchSize) * 0.5f;

    for (int z = 0; z < vertexCountZ; ++z) {
        for (int x = 0; x < vertexCountX; ++x) {
            TerrainVertex v;
            v.position = XMFLOAT3((x * patchSize) - halfWidth, 0.0f, (z * patchSize) - halfDepth);
            v.texcoord = XMFLOAT2((float)x / patchCountX, (float)z / patchCountZ);
            outVertices.push_back(v);
        }
    }

    for (int z = 0; z < patchCountZ; ++z) {
        for (int x = 0; x < patchCountX; ++x) {
            UINT bottomLeft = (z * vertexCountX) + x;
            UINT bottomRight = bottomLeft + 1;
            UINT topLeft = ((z + 1) * vertexCountX) + x;
            UINT topRight = topLeft + 1;

            outIndices.push_back(bottomLeft);
            outIndices.push_back(bottomRight);
            outIndices.push_back(topRight);
            outIndices.push_back(topLeft);
        }
    }
} // GeneratePatchGrid

void Terrain::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    UINT stride = sizeof(TerrainVertex);
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetInputLayout(m_layout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

    m_worldData.world = XMMatrixTranspose(GetWorldMatrix());
    if (!ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_worldData)) {
        return;
    }

    if (!ShaderHelper::UpdateConstantBuffer(context, m_tessellationBuffer.Get(), m_tessellationData)) {
        return;
    }

    if (!ShaderHelper::UpdateConstantBuffer(context, m_heightScaleBuffer.Get(), m_heightSacleData)) {
        return;
    }

    if (!ShaderHelper::UpdateConstantBuffer(context, m_terrainBlendingBuffer.Get(), m_terrainBlending)) {
        return;
    }

    // 상수 버퍼 바인딩
    context->VSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_worldBuffer.GetAddressOf());
    context->DSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_worldBuffer.GetAddressOf());
    context->HSSetConstantBuffers(BUFFER_SLOT_TESS, 1, m_tessellationBuffer.GetAddressOf());
    context->DSSetConstantBuffers(BUFFER_SLOT_HEIGHT, 1, m_heightScaleBuffer.GetAddressOf());
    context->PSSetConstantBuffers(BUFFER_SLOT_BLEND, 1, m_terrainBlendingBuffer.GetAddressOf());

    // 셰이더 바인딩
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->HSSetShader(m_hullShader.Get(), nullptr, 0);
    context->DSSetShader(m_domainShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    ID3D11ShaderResourceView* heightMapSRV = m_heightMap->GetSRV();
    context->DSSetShaderResources(TEX_HEIGHTMAP_SLOT, 1, &heightMapSRV);

    context->DSSetSamplers(LINEAR_SAMPLER_SLOT, 1, &m_linearSampler);
    context->PSSetSamplers(LINEAR_SAMPLER_SLOT, 1, &m_linearSampler);

    context->PSSetShaderResources(TEX_NOR_SLOT, 1, &m_norSRV);
    context->PSSetShaderResources(TEX_SAND_SLOT, 1, &m_sandSRV);
    context->PSSetShaderResources(TEX_GRASS_SLOT, 1, &m_grassSRV);
    context->PSSetShaderResources(TEX_DIFF_SLOT, 1, &m_diffSRV);
    context->PSSetShaderResources(TEX_SNOW_SLOT, 1, &m_snowSRV);

    context->DrawIndexed(m_indexCount, 0, 0);

    context->HSSetShader(nullptr, nullptr, 0);
    context->DSSetShader(nullptr, nullptr, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 기본값으로 복구

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(TEX_NOR_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_SAND_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_GRASS_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_DIFF_SLOT, 1, &nullSRV);
    context->PSSetShaderResources(TEX_SNOW_SLOT, 1, &nullSRV);
} // Render

void Terrain::RenderShadow(ID3D11DeviceContext* context, const RenderShadowParams& params) {
    if (!context) {
        return;
    }

    context->RSSetState(params.states->GetShadowCullBackState());

    UINT stride = sizeof(TerrainVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetInputLayout(m_layout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

    m_worldData.world = XMMatrixTranspose(GetWorldMatrix());
    if (!ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_worldData)) {
        return;
    }
    if (!ShaderHelper::UpdateConstantBuffer(context, m_tessellationBuffer.Get(), m_tessellationData)) {
        return;
    }
    if (!ShaderHelper::UpdateConstantBuffer(context, m_heightScaleBuffer.Get(), m_heightSacleData)) {
        return;
    }

    LightMatrixBuffer lightData;
    lightData.view = XMMatrixTranspose(params.lightView);
    lightData.proj = XMMatrixTranspose(params.lightProj);
    if (!ShaderHelper::UpdateConstantBuffer(context, m_lightMatrixBuffer.Get(), lightData)) {
        return;
    }

    context->VSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_worldBuffer.GetAddressOf());
    context->DSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_worldBuffer.GetAddressOf());
    context->HSSetConstantBuffers(BUFFER_SLOT_TESS, 1, m_tessellationBuffer.GetAddressOf());
    context->DSSetConstantBuffers(BUFFER_SLOT_HEIGHT, 1, m_heightScaleBuffer.GetAddressOf());
    context->DSSetConstantBuffers(BUFFER_SLOT_LIGHT_MATRIX, 1, m_lightMatrixBuffer.GetAddressOf());

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->HSSetShader(m_hullShader.Get(), nullptr, 0);
    context->DSSetShader(m_domainShadowShader.Get(), nullptr, 0);
    context->PSSetShader(nullptr, nullptr, 0);

    ID3D11ShaderResourceView* heightMapSRV = m_heightMap->GetSRV();
    context->DSSetShaderResources(TEX_HEIGHTMAP_SLOT, 1, &heightMapSRV);
    context->DSSetSamplers(LINEAR_SAMPLER_SLOT, 1, &m_linearSampler);

    context->DrawIndexed(m_indexCount, 0, 0);

    context->HSSetShader(nullptr, nullptr, 0);
    context->DSSetShader(nullptr, nullptr, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->DSSetShaderResources(TEX_HEIGHTMAP_SLOT, 1, &nullSRV);
} // RenderShadow

void Terrain::OnGui() {
    if (ImGui::CollapsingHeader("Terrain Settings")) {
        ImGui::SliderFloat("Height Scale", &m_heightSacleData.heightScale, 0.0f, 2000.0f);

        ImGui::SliderFloat("Water Level", &m_terrainBlending.waterLevel, 0.0f, 50.0f);
        ImGui::SliderFloat("Transition Zone", &m_terrainBlending.transZone, 0.1f, 10.0f);
        ImGui::SliderFloat("Grass Limit", &m_terrainBlending.grassLimit, 0.0f, 1.0f);
        ImGui::SliderFloat("Snow Height", &m_terrainBlending.snowHeight, 0.0f, 200.0f);
        ImGui::SliderFloat("UV Tiling", &m_terrainBlending.uv_tiling, 1.0f, 100.0f);
    }
} // OnGui

bool Terrain::InitShader(ID3D11Device* device, HWND hwnd) {
    using namespace ShaderHelper;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (!InitVertexShader(device, hwnd, PathConstants::TERRAIN_VS, 
        layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
        return false;
    }

    if (!InitHullShader(device, hwnd, PathConstants::TERRAIN_HS, m_hullShader.GetAddressOf())) {
        return false;
    }

    if (!InitDomainShader(device, hwnd, PathConstants::TERRAIN_DS, m_domainShader.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(device, hwnd, PathConstants::TERRAIN_PS, m_pixelShader.GetAddressOf())) {
        return false;
    }

    if (!InitDomainShader(device, hwnd, PathConstants::TERRAIN_DEPTH_DS, m_domainShadowShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<LightMatrixBuffer>(device, m_lightMatrixBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<WorldBuffer>(device, m_worldBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<TessellationControlBuffer>(device, m_tessellationBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<HeightScaleBuffer>(device, m_heightScaleBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<TerrainBlendingBuffer>(device, m_terrainBlendingBuffer.GetAddressOf())) {
        return false;
    }
    return true;
} // InitShader

DirectX::XMMATRIX Terrain::GetWorldMatrix() {
    return XMMatrixIdentity();
} // GetWorldMatrix

float Terrain::GetHeightAt(float worldX, float worldZ) const {
    if (!m_heightMap) {
        return 0.0f;
    }

    float totalWidth = m_patchCountX * m_patchSize;
    float totalDepth = m_patchCountZ * m_patchSize;
    float halfWidth = totalWidth * 0.5f;
    float halfDepth = totalDepth * 0.5f;

    float u = (worldX + halfWidth) / totalWidth;
    float v = (worldZ + halfDepth) / totalDepth;

    if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f) {
        return 0.0f;
    }

    int texX = static_cast<int>(u * (m_heightMap->GetWidth() - 1));
    int texY = static_cast<int>(v * (m_heightMap->GetHeight() - 1));

    float height = m_heightMap->GetPixelHeight(texX, texY);
    return height * m_heightSacleData.heightScale;
} // GetHeightAt

float Terrain::GetWidth() const { return m_patchCountX * m_patchSize; }
float Terrain::GetDepth() const { return m_patchCountZ * m_patchSize; }