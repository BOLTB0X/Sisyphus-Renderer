#include "Pch.h"
#include "Texture.h"
#include "TextureLoader.h"
// Utils
#include "Helpers/DebugHelper.h"

Texture::Texture() {
}; // Texture

Texture::~Texture() {
}; // ~Texture

bool Texture::Init(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path) {
	return TextureLoader::CreateTextureFromFile(device, context, path, m_srv.GetAddressOf());
} // Init

ID3D11ShaderResourceView* Texture::GetSRV() const {
    return m_srv.Get();
} // GetSRV