#include "Pch.h"
#include "TextureManager.h"
// Resources
#include "Resources/Texture.h"
#include "Resources/VolumeTexture.h"
// Data
#include "Data/NoiseGenerator.h"
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
	NoiseGenerator::InitParams initParams;
    initParams.device = device;
    initParams.hwnd = hwnd;
    initParams.path = PathConstants::WORLEY_NOISE_CS;
    initParams.groupSize = 8;
    if (!m_NoiseGenerator->Init(initParams)) {
        return false;
    }

	LoadTexture(device, context, PathConstants::BLUE_NOISE);
	LoadTexture(device, context, PathConstants::NOISE_2D);
	LoadTexture(device, context, PathConstants::HEIGHT);

    CreateVolumeTexture(device, PathConstants::KEY_WORLEY_NOISE, 32, 32, 32, DXGI_FORMAT_R16G16B16A16_FLOAT);
    CreateCloudNoise(context, PathConstants::KEY_WORLEY_NOISE);
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

void TextureManager::CreateCloudNoise(ID3D11DeviceContext* context, const std::string& name) {
    auto volumeTex = GetVolumeTexture(name);

    if (volumeTex && m_NoiseGenerator) {
		NoiseGenerator::GenerateParams params;
		params.target = volumeTex.get();
		params.resolution = DirectX::XMFLOAT3(128.0f, 128.0f, 128.0f);
        m_NoiseGenerator->Generate(context, params);
        context->GenerateMips(volumeTex->GetSRV());
    }

} // CreateNoise

void TextureManager::LoadTexture(
    ID3D11Device* device,
    ID3D11DeviceContext* context,
    const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_Textures.find(filename);
    if (it != m_Textures.end()) {
        return;
    }

    auto newTexture = std::make_shared<Texture>();
    if (newTexture->Init(device, context, filename)) {
        m_Textures[filename] = newTexture;
    }
} // LoadTexture

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