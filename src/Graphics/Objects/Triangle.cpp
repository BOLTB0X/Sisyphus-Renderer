#include "Pch.h"
#include "Triangle.h"
#include "Resources/VertexTypes.h"
#include "Resources/ConstantBufferType.h"
#include "Shaders/ColorShader.h"
#include "Utils/SharedConstants.h"

using namespace SharedConstants::HLSL;
using namespace DirectX;

Triangle::Triangle()
    : m_vertexCount(0), m_indexCount(0) {
    m_colorShader = std::make_unique<ColorShader>();
} // Triangle

Triangle::~Triangle() {
    Shutdown();
} // ~Triangle

bool Triangle::Init(ID3D11Device* device, HWND hwnd) {
    if (!InitBuffers(device)) {
        return false;
    }

    if (!m_colorShader->Init(device, hwnd, COLOR_VS, COLOR_PS)) {
        return false;
    }

    return true;
} // Init

bool Triangle::InitBuffers(ID3D11Device* device) {
    Vertex::VertexColor vertices[] = {
        { {  0.0f,   0.433f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, // 위
        { {  0.5f,  -0.433f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // 우하
        { { -0.5f,  -0.433f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }  // 좌하
    };
    m_vertexCount = sizeof(vertices) / sizeof(vertices[0]);

    unsigned long indices[] = { 0, 1, 2 };
    m_indexCount = sizeof(indices) / sizeof(indices[0]);;

    // Vertex Buffer 생성
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(Vertex::VertexColor) * m_vertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vsd = {};
    vsd.pSysMem = vertices;
    if (FAILED(device->CreateBuffer(&vbd, &vsd, m_vertexBuffer.GetAddressOf()))) {
        return false;
    }

    // Index Buffer 생성
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(unsigned long) * m_indexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA isd = {};
    isd.pSysMem = indices;
    if (FAILED(device->CreateBuffer(&ibd, &isd, m_indexBuffer.GetAddressOf()))) {
        return false;
    }
    return true;
} // InitBuffers


void Triangle::Render(ID3D11DeviceContext* context) {
    // 행렬 생성
    XMMATRIX world = XMMatrixIdentity();
    XMMATRIX view = XMMatrixIdentity();
    XMMATRIX projection = XMMatrixIdentity();

    RenderBuffers(context);

    if (m_colorShader) {
        m_colorShader->Render(context, m_indexCount, world, view, projection);
    }
} // Render

void Triangle::RenderBuffers(ID3D11DeviceContext* context) {
    UINT stride = sizeof(Vertex::VertexColor);
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
} // RenderBuffers

void Triangle::Shutdown() {
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();

    if (m_colorShader) {
        m_colorShader.reset();
    }
} // Shutdown