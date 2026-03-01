#pragma once
#include <d3d11.h>
#include <string>

class TextureLoader {
public:
    static bool CreateTextureFromFile(ID3D11Device*, ID3D11DeviceContext*,
        const std::string&, ID3D11ShaderResourceView**);
}; // TextureLoader