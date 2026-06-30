// TextureManager.h
#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <future>
#include <mutex>

class Texture;
class VolumeTexture;
class NoiseGenerator;

class TextureManager {
public:
    TextureManager();
    TextureManager(const TextureManager&) = delete;
    ~TextureManager();

    bool Init(ID3D11Device*, ID3D11DeviceContext*, HWND);
    void Shutdown();
    void CreateVolumeTexture(ID3D11Device*, const std::string&, UINT, UINT, UINT, DXGI_FORMAT);
    void CreateCloudNoise(ID3D11DeviceContext*, const std::string&, NoiseGenerator*, float);

    std::shared_ptr<Texture>       GetTexture(ID3D11Device*, ID3D11DeviceContext*, const std::string&, bool keepCpuPixels = false);
    std::shared_ptr<VolumeTexture> GetVolumeTexture(const std::string&);

private:
    void LoadTexture(ID3D11Device*, ID3D11DeviceContext*, const std::string&, bool keepCpuPixels = false);

private:
    std::unordered_map<std::string, std::shared_ptr<Texture>>       m_Textures;
    std::unordered_map<std::string, std::shared_ptr<VolumeTexture>> m_VolumeTextures;
    std::unique_ptr<NoiseGenerator>                                 m_PerlinGenerator;
    std::unique_ptr<NoiseGenerator>                                 m_WorleyGenerator;
    std::mutex                                                      m_mutex;
}; // TexturesManager