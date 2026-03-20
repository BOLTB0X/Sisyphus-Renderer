#include "Pch.h"
#include "RenderTexture.h"

RenderTexture::RenderTexture() : m_width(0), m_height(0) {
} // RenderTexture

RenderTexture::~RenderTexture() {
} // ~RenderTexture

bool RenderTexture::Init(ID3D11Device* device, int width, int height, DXGI_FORMAT format) {
    m_width = width;
    m_height = height;

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, m_texture.GetAddressOf()))) {
        return false;
    }

    if (FAILED(device->CreateRenderTargetView(m_texture.Get(), nullptr, m_rtv.GetAddressOf()))) {
        return false;
    }

    if (FAILED(device->CreateShaderResourceView(m_texture.Get(), nullptr, m_srv.GetAddressOf()))) {
        return false;
    }

    return true;
} // Init

bool RenderTexture::InitDepth(ID3D11Device* device, int width, int height) {
    m_width = width;
    m_height = height;

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, m_texture.GetAddressOf()))) {
        return false;
    }

    // GPU가 깊이를 기록할 때 바라보는 뷰 (DSV)
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // 깊이 데이터로 해석
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    if (FAILED(device->CreateDepthStencilView(m_texture.Get(), &dsvDesc, m_dsv.GetAddressOf()))) {
        return false;
    }

    // 픽셀 셰이더가 값을 읽어갈 때 바라보는 뷰 (SRV)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    if (FAILED(device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srv.GetAddressOf()))) {
        return false;
    }

    return true;
} // InitDepth

void RenderTexture::Clear(ID3D11DeviceContext* context, float r, float g, float b, float a) {
    if (m_rtv) {
        float color[4] = { r, g, b, a };
        context->ClearRenderTargetView(m_rtv.Get(), color);
    }
} // Clear

void RenderTexture::ClearDepth(ID3D11DeviceContext* context, float depth, UINT8 stencil) {
    if (m_dsv) {
        context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH, depth, stencil);
    }
} // ClearDepth

ID3D11ShaderResourceView* RenderTexture::GetSRV() const { return m_srv.Get(); }
ID3D11RenderTargetView*   RenderTexture::GetRTV() const { return m_rtv.Get(); }
ID3D11DepthStencilView*   RenderTexture::GetDSV() const { return m_dsv.Get(); }
int                       RenderTexture::GetWidth() const { return m_width; }
int                       RenderTexture::GetHeight() const { return m_height; }