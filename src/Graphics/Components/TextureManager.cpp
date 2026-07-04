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
    m_PerlinGenerator = std::make_unique<NoiseGenerator>();
    m_WorleyGenerator = std::make_unique<NoiseGenerator>();
} // TextureManager

TextureManager::~TextureManager() {
    Shutdown();
} // ~TextureManager

bool TextureManager::Init(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) {
	NoiseGenerator::InitParams initParams;
	LoadTexture(device, context, PathConstants::BLUE_NOISE);
	LoadTexture(device, context, PathConstants::NOISE_2D);
	//LoadTexture(device, context, PathConstants::HEIGHT, true);
	LoadTexture(device, context, PathConstants::GRASS);
	LoadTexture(device, context, PathConstants::GROUND_COL);
	LoadTexture(device, context, PathConstants::GROUND_NOR);

	LoadTexture(device, context, PathConstants::WATER_NOR);
	LoadTexture(device, context, PathConstants::WATER_WAVE_NOR);
	LoadTexture(device, context, PathConstants::FLOW_MAP);

	LoadTexture(device, context, PathConstants::TERRAIN_COL);
	LoadTexture(device, context, PathConstants::TERRAIN_RNOL);
	LoadTexture(device, context, PathConstants::TERRAIN_RDIFF);
	LoadTexture(device, context, PathConstants::TERRAIN_GRASS);
	LoadTexture(device, context, PathConstants::TERRAIN_SNOW);
	LoadTexture(device, context, PathConstants::TERRAIN_SAND);

    NoiseGenerator::InitParams worleyParams;
    worleyParams.device = device;
    worleyParams.hwnd = hwnd;
    worleyParams.path = PathConstants::WORLEY_NOISE_CS;
    worleyParams.groupSize = 8;
    if (!m_WorleyGenerator->Init(worleyParams)) {
        return false;
    }

    CreateVolumeTexture(device, PathConstants::KEY_WORLEY_NOISE, 32, 32, 32, DXGI_FORMAT_R16G16B16A16_FLOAT);
    CreateCloudNoise(context, PathConstants::KEY_WORLEY_NOISE, m_WorleyGenerator.get(), 32.0f);

    NoiseGenerator::InitParams perlinParams;
    perlinParams.device = device;
    perlinParams.hwnd = hwnd;
    perlinParams.path = PathConstants::PERLINE_CS;
    perlinParams.groupSize = 8;

    if (!m_PerlinGenerator->Init(perlinParams)) {
        return false;
    }
    else {
        auto generatedHeightMap = CreateProceduralHeightMap(device, context, PathConstants::HEIGHT, 1024, 1024);
        m_Textures[PathConstants::HEIGHT] = generatedHeightMap;
    }

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

void TextureManager::CreateCloudNoise(ID3D11DeviceContext* context, const std::string& name, NoiseGenerator* generator, float resolution) {
    auto volumeTex = GetVolumeTexture(name);

    if (volumeTex && generator) {
        NoiseGenerator::Generate3DParams params;
        params.target = volumeTex.get();
        params.resolution = DirectX::XMFLOAT3(resolution, resolution, resolution);

        generator->GenerateVolume(context, params);
        context->GenerateMips(volumeTex->GetSRV());
    }
} // CreateCloudNoise

void TextureManager::LoadTexture(
    ID3D11Device* device,
    ID3D11DeviceContext* context,
    const std::string& filename,
    bool keepCpuPixels) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_Textures.find(filename);
    if (it != m_Textures.end()) {
        return;
    }

    auto newTexture = std::make_shared<Texture>();
    if (newTexture->Init(device, context, filename, keepCpuPixels)) {
        m_Textures[filename] = newTexture;
    }
} // LoadTexture

std::shared_ptr<Texture> TextureManager::CreateProceduralHeightMap(
    ID3D11Device* device, ID3D11DeviceContext* context,
    const std::string& name,  UINT width, UINT height)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto newTexture = std::make_shared<Texture>();

    NoiseGenerator::Generate2DParams params;
    params.device = device;
    params.width = width;
    params.height = height;
    params.outputTexture = newTexture.get();

    m_PerlinGenerator->Generate2D(context, params);
    m_Textures[name] = newTexture;
    return newTexture;
} // CreateProceduralHeightMap

std::shared_ptr<Texture> TextureManager::GetTexture(
    ID3D11Device* device,
    ID3D11DeviceContext* context,
    const std::string& filename,
    bool keepCpuPixels) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_Textures.find(filename);
    if (it != m_Textures.end())
        return it->second;

    auto newTexture = std::make_shared<Texture>();
    if (newTexture->Init(device, context, filename, keepCpuPixels)) {
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