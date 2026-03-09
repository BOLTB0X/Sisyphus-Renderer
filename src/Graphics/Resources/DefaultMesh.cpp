#include "Pch.h"
#include "DefaultMesh.h"
#include <DirectXMath.h>
// Utils
#include "Helpers/MathHelper.h"
#include "Helpers/DebugHelper.h"

using namespace DirectX;
using namespace MathHelper;

DefaultMesh::DefaultMesh() : m_vertexCount(0), m_indexCount(0) {
} // DefaultMesh

DefaultMesh::~DefaultMesh() {
} // ~DefaultMesh

bool DefaultMesh::Init(ID3D11Device* device, UINT scale, DefaultMeshType type) {
    if (type == DefaultMeshType::Cube) {
        if (!InitCube(device, scale)) {
            return false;
        }
    } else if (type == DefaultMeshType::Quad) {
        if (!InitQuad(device, scale)) {
            return false;
        }
    } else {
        return false;
	}

    return true;
} // Init

void DefaultMesh::RenderBuffer(ID3D11DeviceContext* context) {
    UINT stride = sizeof(DefaultMesh::BoxVertex);
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
} // Render

UINT DefaultMesh::GetVertexCount() const { return m_vertexCount; }
UINT DefaultMesh::GetIndexCount() const { return m_indexCount; }

bool DefaultMesh::InitCube(ID3D11Device* device, UINT scale) {
    float s = (float)scale;
    XMFLOAT3 p[8] = {
        {-s,  s, -s}, { s,  s, -s}, {-s, -s, -s}, { s, -s, -s}, // Front: TL, TR, BL, BR (0,1,2,3)
        {-s,  s,  s}, { s,  s,  s}, {-s, -s,  s}, { s, -s,  s}  // Back:  TL, TR, BL, BR (4,5,6,7)
    };

    std::vector<BoxVertex> vertices = {
        // Front face (Z = -s)
        { p[0], {0.0f, 0.0f} }, { p[1], {1.0f, 0.0f} }, { p[2], {0.0f, 1.0f} }, { p[3], {1.0f, 1.0f} },
        // Back face (Z = +s) 
        { p[5], {0.0f, 0.0f} }, { p[4], {1.0f, 0.0f} }, { p[7], {0.0f, 1.0f} }, { p[6], {1.0f, 1.0f} },
        // Top face (Y = +s)
        { p[4], {0.0f, 0.0f} }, { p[5], {1.0f, 0.0f} }, { p[0], {0.0f, 1.0f} }, { p[1], {1.0f, 1.0f} },
        // Bottom face (Y = -s)
        { p[2], {0.0f, 0.0f} }, { p[3], {1.0f, 0.0f} }, { p[6], {0.0f, 1.0f} }, { p[7], {1.0f, 1.0f} },
        // Left face (X = -s)
        { p[4], {0.0f, 0.0f} }, { p[0], {1.0f, 0.0f} }, { p[6], {0.0f, 1.0f} }, { p[2], {1.0f, 1.0f} },
        // Right face (X = +s)
        { p[1], {0.0f, 0.0f} }, { p[5], {1.0f, 0.0f} }, { p[3], {0.0f, 1.0f} }, { p[7], {1.0f, 1.0f} }
    };

    m_vertexCount = static_cast<UINT>(vertices.size());

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < 6; i++) {
        int offset = i * 4;
        indices.push_back(offset + 0); indices.push_back(offset + 1); indices.push_back(offset + 2);
        indices.push_back(offset + 2); indices.push_back(offset + 1); indices.push_back(offset + 3);
    }
    m_indexCount = static_cast<UINT>(indices.size());

    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(BoxVertex) * static_cast<UINT>(vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData = {};
    vInitData.pSysMem = vertices.data();

    HRESULT hr = device->CreateBuffer(&vbd, &vInitData, m_vertexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * m_indexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData = {};
    iInitData.pSysMem = indices.data();

    hr = device->CreateBuffer(&ibd, &iInitData, m_indexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }
    DebugHelper::DebugPrint("DefaultMesh 생성 완료 - Vertex: " + std::to_string(m_vertexCount) +
        ", Index: " + std::to_string(m_indexCount));
    return true;
} // InitCube

bool DefaultMesh::InitQuad(ID3D11Device* device, UINT scale) {
    float s = (float)scale;
    XMFLOAT3 p[4] = {
        {-s,  s, 0.0f}, { s,  s, 0.0f}, {-s, -s, 0.0f}, { s, -s, 0.0f} // TL, TR, BL, BR
    };
    std::vector<BoxVertex> vertices = {
        { p[0], {0.0f, 0.0f} }, { p[1], {1.0f, 0.0f} }, { p[2], {0.0f, 1.0f} }, { p[3], {1.0f, 1.0f} }
    };

    m_vertexCount = static_cast<UINT>(vertices.size());
    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 1, 3
    };
    m_indexCount = static_cast<UINT>(indices.size());

    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(BoxVertex) * static_cast<UINT>(vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData = {};
    vInitData.pSysMem = vertices.data();
    HRESULT hr = device->CreateBuffer(&vbd, &vInitData, m_vertexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * m_indexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA iInitData = {};
    iInitData.pSysMem = indices.data();

    hr = device->CreateBuffer(&ibd, &iInitData, m_indexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    DebugHelper::DebugPrint("DefaultMesh 생성 완료 - Vertex: " + std::to_string(m_vertexCount) +
        ", Index: " + std::to_string(m_indexCount));
    return true;
} // InitQuad