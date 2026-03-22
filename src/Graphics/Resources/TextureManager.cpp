#include "Pch.h"
#include "TextureManager.h"
#include "Texture.h"
#include "VolumeTexture.h"
#include "NoiseGenerator.h"
// Utils
#include "Helpers/DebugHelper.h"
#include "SharedConstants/PathConstants.h"
#include "Helpers/ShaderHelper.h"

using namespace DebugHelper;
using namespace SharedConstants;

TextureManager::TextureManager() {
    m_Textures = std::unordered_map<std::string, std::shared_ptr<Texture>>();
    m_VolumeTextures = std::unordered_map<std::string, std::shared_ptr<VolumeTexture>>();
    m_NoiseGenerator = std::make_unique<NoiseGenerator>();
} // TextureManager

TextureManager::~TextureManager() {
    Shutdown();
} // ~TextureManager

bool TextureManager::Init(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) {
    if (!m_NoiseGenerator->Init(device, hwnd, PathConstants::NOISEGEN_CS)) {
        return false;
    }

    auto newTexture = std::make_shared<Texture>();
    if (newTexture->Init(device, context, PathConstants::WMAP)) {
        m_Textures[PathConstants::WMAP] = newTexture;
    }

    CreateVolumeTexture(device, PathConstants::KEY_CLOUD_VOL, 128, 128, 128, DXGI_FORMAT_R16G16B16A16_FLOAT);

    NoiseGenerator::NoiseBuffer noiseParams = NoiseGenerator::NoiseBuffer();
    CreateCloudNoise(context, PathConstants::KEY_CLOUD_VOL, noiseParams);
    return true;
} // Init

void TextureManager::Shutdown() {
    m_Textures.clear();
    m_VolumeTextures.clear();
} // Shutdown

void TextureManager::CreateVolumeTexture(
    ID3D11Device* device, const std::string& name,
    UINT width, UINT height, UINT depth, DXGI_FORMAT format) {

    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_VolumeTextures.find(name);
    if (it != m_VolumeTextures.end()) {
        return;
    }

    auto newVolTex = std::make_shared<VolumeTexture>();
    if (newVolTex->Init(device, width, height, depth, format)) {
        m_VolumeTextures[name] = newVolTex;
        return;
    }
} // CreateVolumeTexture

void TextureManager::CreateCloudNoise(ID3D11DeviceContext* context, const std::string& name, const NoiseGenerator::NoiseBuffer& noiseParams) {
    auto volumeTex = GetVolumeTexture(name);
    if (volumeTex && m_NoiseGenerator) {
        m_NoiseGenerator->Generate(context, volumeTex.get(), noiseParams);
    }
} // CreateNoise

std::shared_ptr<Texture> TextureManager::GetTexture(
    ID3D11Device* device,
    ID3D11DeviceContext* context,
    const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_Textures.find(filename);
    if (it != m_Textures.end())
        return it->second;

    auto newTexture = std::make_shared<Texture>();
    if (newTexture->Init(device, context, filename)) {
        m_Textures[filename] = newTexture;
        return newTexture;
    }

    return nullptr;
} // GetTexture

std::shared_ptr<VolumeTexture> TextureManager::GetVolumeTexture(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_VolumeTextures.find(name);
    if (it != m_VolumeTextures.end()) {
        return it->second;
    }
    return nullptr;
} // GetVolumeTexture