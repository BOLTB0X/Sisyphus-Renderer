#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>


class Texture {
public:
    Texture();
    Texture(const Texture&) = delete;
    ~Texture();

    bool                      Init(ID3D11Device*, ID3D11DeviceContext*, const std::string&);
    ID3D11ShaderResourceView* GetSRV() const;

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
}; // Texture