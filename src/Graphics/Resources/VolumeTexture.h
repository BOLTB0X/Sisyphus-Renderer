#pragma once
#include <d3d11.h>
#include <wrl/client.h>

class VolumeTexture {
public:
    VolumeTexture();
    ~VolumeTexture();

    bool Init(ID3D11Device*, UINT, UINT, UINT, DXGI_FORMAT);

    ID3D11ShaderResourceView*  GetSRV() const;
    ID3D11UnorderedAccessView* GetUAV() const;
    ID3D11Texture3D*           GetTexture() const;

private:
    Microsoft::WRL::ComPtr<ID3D11Texture3D>             m_texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_srv;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>   m_uav;

    UINT m_width;
    UINT m_height;
    UINT m_depth;
}; // VolumeTexture