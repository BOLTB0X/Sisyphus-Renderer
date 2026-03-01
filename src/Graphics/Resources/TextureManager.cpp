#include "Pch.h"
#include "TextureManager.h"
#include "Texture.h"
// Utils
#include "Helpers/DebugHelper.h"

using namespace DebugHelper;

TextureManager::TextureManager() {
    m_Textures = std::unordered_map<std::string, std::shared_ptr<Texture>>();
} // TextureManager

TextureManager::~TextureManager() {
    Shutdown();
} // ~TextureManager

bool TextureManager::Init(ID3D11Device* device, ID3D11DeviceContext* context) {

    return true;
} // Init

void TextureManager::Shutdown() {
    m_Textures.clear();
} // Shutdown

std::shared_ptr<Texture> TextureManager::GetTexture(
    ID3D11Device* device,
    ID3D11DeviceContext* context,
    const std::string& filename) {
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
