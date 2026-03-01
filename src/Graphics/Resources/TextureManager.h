// TextureManager.h
#pragma once
#include <d3d11.h>
#include <unordered_map>
#include <string>
#include <memory>

class Texture;

class TextureManager {
public:
    TextureManager();
    TextureManager(const TextureManager&) = delete;
    ~TextureManager();

    bool                     Init(ID3D11Device*, ID3D11DeviceContext*);
    void                     Shutdown();
    std::shared_ptr<Texture> GetTexture(ID3D11Device*, ID3D11DeviceContext*, const std::string&);

private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;
}; // TexturesManager