#include "Pch.h"
#include "Triangle.h"
#include "Resources/ConstantBufferType.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "Helpers/ShaderHelper.h"

using namespace SharedConstants::PathConstants;
using namespace DirectX;

Triangle::Triangle()
    : m_vertexCount(0), m_indexCount(0) {
} // Triangle

Triangle::~Triangle() {
    Shutdown();
} // ~Triangle

bool Triangle::Init(ID3D11Device* device, HWND hwnd) {
    using namespace ShaderHelper;

    if (!InitBuffers(device)) {
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    if (!InitVertexShader(device, hwnd, COLOR_VS,
        layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(device, hwnd, COLOR_PS, m_pixelShader.GetAddressOf())) {
        return false;
	}

    if (!InitConstantBuffer<ConstantBuffer::MatrixBuffer>(device, m_matrixBuffer.GetAddressOf())) {
        return false;
	}

    return true;
} // Init

bool Triangle::InitBuffers(ID3D11Device* device) {
    ColorVertex vertices[] = {
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
    vbd.ByteWidth = sizeof(ColorVertex) * m_vertexCount;
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

void Triangle::Shutdown() {
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
} // Shutdown


void Triangle::Render(ID3D11DeviceContext* context) {
    // 행렬 생성
    XMMATRIX world = XMMatrixIdentity();
    XMMATRIX view = XMMatrixIdentity();
    XMMATRIX projection = XMMatrixIdentity();

    RenderBuffers(context);
    RenderShader(context, m_indexCount, world, view, projection);
} // Render

void Triangle::RenderBuffers(ID3D11DeviceContext* context) {
    UINT stride = sizeof(ColorVertex);
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
} // RenderBuffers

void Triangle::RenderShader(ID3D11DeviceContext* context, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection) {
    using namespace ShaderHelper;

    ConstantBuffer::MatrixBuffer buffer;
    buffer.world = XMMatrixTranspose(world);
    buffer.view = XMMatrixTranspose(view);
    buffer.projection = XMMatrixTranspose(projection);

    if (!UpdateConstantBuffer(context, m_matrixBuffer.Get(), buffer)) {
        return;
    }

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    context->IASetInputLayout(m_layout.Get());

    context->DrawIndexed(indexCount, 0, 0);
} // RenderShader
