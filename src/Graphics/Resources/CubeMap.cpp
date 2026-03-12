#include "Pch.h"
#include "CubeMap.h"
// Utils
#include "Helpers/DebugHelper.h"

using namespace DirectX;

CubeMap::CubeMap() : m_width(0), m_height(0),  m_indexCount(0) {
    for (int i = 0; i < 6; ++i) { m_viewMatrices[i] = XMMatrixIdentity(); }
    m_projMatrix = XMMatrixIdentity();
} // CubeMap

CubeMap::~CubeMap() {
} // ~CubeMap

void CubeMap::Init(ID3D11Device* device, UINT width, UINT height) {
	m_width = width;
	m_height = height;

	CreateRenderTarget(device);
	CreateGeometry(device);
	InitMatrices();
} // Init

void CubeMap::RenderBuffer(ID3D11DeviceContext* context) {
	UINT stride = sizeof(CubeVertex);
	UINT offset = 0;

	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
} // RenderBuffer

ID3D11ShaderResourceView* CubeMap::GetSRV() const { return m_colorSRV.Get(); }
ID3D11RenderTargetView*   CubeMap::GetRTV(int face) const { return m_colorRTV[face].Get(); }
const DirectX::XMMATRIX&  CubeMap::GetViewMatrix(int face) const { return m_viewMatrices[face]; }
const DirectX::XMMATRIX&  CubeMap::GetProjMatrix() const { return m_projMatrix; }
UINT                      CubeMap::GetIndexCount() const { return m_indexCount; }

void CubeMap::CreateRenderTarget(ID3D11Device* device) {
    // 큐브맵 텍스처 생성
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = m_width;
    textureDesc.Height = m_height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 6;
    textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // HDR 포맷
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // 필수

    device->CreateTexture2D(&textureDesc, nullptr, &m_colorTexture);

    // 각 Face에 대한 RenderTargetView 생성
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = textureDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    rtvDesc.Texture2DArray.MipSlice = 0;
    rtvDesc.Texture2DArray.ArraySize = 1; // 한 번에 한 면씩

    for (int i = 0; i < 6; ++i) {
        rtvDesc.Texture2DArray.FirstArraySlice = i;
        device->CreateRenderTargetView(m_colorTexture.Get(), &rtvDesc, &m_colorRTV[i]);
    }

    // 셰이더에서 샘플링할 ShaderResourceView 생성
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = 1;
    srvDesc.TextureCube.MostDetailedMip = 0;

    device->CreateShaderResourceView(m_colorTexture.Get(), &srvDesc, &m_colorSRV);
} // CreateRenderTarget

void CubeMap::CreateGeometry(ID3D11Device* device) {
    // 8개의 버텍스 정의
    CubeVertex vertices[] = {
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f,  1.0f, -1.0f),  XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f),  XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f,  1.0f,  1.0f),  XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f,  1.0f),  XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) }
    };

    // 안쪽을 바라보도록 인덱스 설정
    UINT indices[] = {
        0, 1, 2, 0, 2, 3, // Front
        1, 5, 6, 1, 6, 2, // Right
        5, 4, 7, 5, 7, 6, // Back
        4, 0, 3, 4, 3, 7, // Left
        4, 5, 1, 4, 1, 0, // Top
        3, 2, 6, 3, 6, 7  // Bottom
    };

    m_indexCount = ARRAYSIZE(indices);

    // Vertex Buffer
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(vertices);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vInitData = { vertices };
    device->CreateBuffer(&vbd, &vInitData, &m_vertexBuffer);

    // Index Buffer
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(indices);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iInitData = { indices };
    device->CreateBuffer(&ibd, &iInitData, &m_indexBuffer);
} // CreateGeometry

void CubeMap::InitMatrices() {
    XMVECTOR center = XMVectorSet(0, 0, 0, 0);
    // +X, -X, +Y, -Y, +Z, -Z 순서
    XMVECTOR targets[6] = {
        XMVectorSet(1, 0, 0, 0), XMVectorSet(-1, 0, 0, 0),
        XMVectorSet(0, 1, 0, 0), XMVectorSet(0, -1, 0, 0),
        XMVectorSet(0, 0, 1, 0), XMVectorSet(0, 0, -1, 0)
    };
    XMVECTOR ups[6] = {
        XMVectorSet(0, 1, 0, 0), XMVectorSet(0, 1, 0, 0),
        XMVectorSet(0, 0, -1, 0), XMVectorSet(0, 0, 1, 0),
        XMVectorSet(0, 1, 0, 0), XMVectorSet(0, 1, 0, 0)
    };

    for (int i = 0; i < 6; ++i) {
        m_viewMatrices[i] = XMMatrixLookAtLH(center, targets[i], ups[i]);
    }
    
    m_projMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, 0.1f, 10.0f);
} // InitMatrices