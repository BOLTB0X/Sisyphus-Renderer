#include "Pch.h"
#include "VolumeTexture.h"
#include "Helpers/DebugHelper.h"

VolumeTexture::VolumeTexture() : 
    m_width(0), m_height(0), m_depth(0) {
} // VolumeTexture

VolumeTexture::~VolumeTexture() {
} // ~VolumeTexture

bool VolumeTexture::Init(ID3D11Device* device, UINT width, UINT height, UINT depth, DXGI_FORMAT format) {
    m_width = width;
    m_height = height;
    m_depth = depth;

    // 3D 텍스처 생성
    D3D11_TEXTURE3D_DESC texDesc = {};
    texDesc.Width = m_width;
    texDesc.Height = m_height;
    texDesc.Depth = m_depth;
    texDesc.MipLevels = 1;
    texDesc.Format = format;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    // SRV(읽기)와 UAV(쓰기) 권한을 모두 부여
    // 구름 밀도는 컴퓨트 셰이더에서 기록되고, 레이마칭 셰이더에서 읽히므로 둘 다 필요
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS; 

    HRESULT hr = device->CreateTexture3D(&texDesc, nullptr, m_texture.GetAddressOf());
    if (FAILED(hr)) {
        DebugHelper::DebugPrint("VolumeTexture 3D 텍스처 생성 실패");
        return false;
    }

    // SRV 생성 (레이마칭 셰이더에서 밀도를 읽을 때 사용)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
    srvDesc.Texture3D.MostDetailedMip = 0;
    srvDesc.Texture3D.MipLevels = 1;

    hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srv.GetAddressOf());
    if (FAILED(hr)) return false;

    // UAV 생성
    // 지금 당장 컴퓨트 셰이더가 노이즈를 기록할 때 사용
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
    uavDesc.Texture3D.MipSlice = 0;
    uavDesc.Texture3D.FirstWSlice = 0;
    uavDesc.Texture3D.WSize = m_depth;

    hr = device->CreateUnorderedAccessView(m_texture.Get(), &uavDesc, m_uav.GetAddressOf());
    if (FAILED(hr)) return false;

    return true;
} // Init

ID3D11ShaderResourceView*  VolumeTexture::GetSRV() const { return m_srv.Get(); }
ID3D11UnorderedAccessView* VolumeTexture::GetUAV() const { return m_uav.Get(); }
ID3D11Texture3D*           VolumeTexture::GetTexture() const { return m_texture.Get(); }