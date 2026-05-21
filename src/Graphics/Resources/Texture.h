#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <vector>

class Texture {
public:
    Texture();
    Texture(const Texture&) = delete;
    ~Texture();

    bool                      Init(ID3D11Device*, ID3D11DeviceContext*, const std::string&, bool keepCpuPixels = false);
    ID3D11ShaderResourceView* GetSRV() const;
    float                     GetPixelHeight(int, int) const;
    int                       GetWidth() const;
	int 					  GetHeight() const;

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
    std::vector<unsigned char>                       m_cpuPixels;
    int                                              m_width;
    int                                              m_height;
}; // Texture