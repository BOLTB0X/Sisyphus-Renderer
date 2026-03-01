#include "Pch.h"
#include "PBRMesh.h"

PBRMesh::PBRMesh()
: m_indexCount(0),
  m_materialIndex(0),
  m_stride(sizeof(VertexTypes::FBRVertex)),
  m_offset(0) {
} // PBRMesh

PBRMesh::~PBRMesh() {
} // ~PBRMesh

bool PBRMesh::Init(ID3D11Device* device,
    const std::vector<VertexTypes::FBRVertex>& vertices,
    const std::vector<unsigned int>& indices,
    unsigned int materialIndex) {
    m_indexCount = static_cast<UINT>(indices.size());
    m_materialIndex = materialIndex;

    // Vertex Buffer 생성
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(VertexTypes::FBRVertex) * static_cast<UINT>(vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vinitData = {};
    vinitData.pSysMem = vertices.data();

    HRESULT hr = device->CreateBuffer(&vbd, &vinitData, m_vertexBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    // Index Buffer 생성
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(unsigned int) * m_indexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA iinitData = {};
    iinitData.pSysMem = indices.data();

    hr = device->CreateBuffer(&ibd, &iinitData, m_indexBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    return true;
} // Init

void PBRMesh::RenderBuffer(ID3D11DeviceContext* context) {
    // 입력 조립기 세팅
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_stride, &m_offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    
    // 프리미티브 토폴로지 설정
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 드로우 콜
    context->DrawIndexed(m_indexCount, 0, 0);
} // Render

unsigned int PBRMesh::GetMaterialIndex() const { return m_materialIndex; }
UINT         PBRMesh::GetIndexCount() const { return m_indexCount; }

