#include "Pch.h"
#include "Texture.h"
#include "TextureLoader.h"
// Utils
#include "Helpers/DebugHelper.h"
// STL
#include <algorithm>

Texture::Texture() {
	m_width = 0;
	m_height = 0;
}; // Texture

Texture::~Texture() {
}; // ~Texture

bool Texture::Init(ID3D11Device* device,  ID3D11DeviceContext* context,
    const std::string& path, bool keepCpuPixels) {
    if (keepCpuPixels) {
        return TextureLoader::CreateTextureFromFile(device, context, path, m_srv.GetAddressOf(), &m_cpuPixels, &m_width, &m_height);
    }

    return TextureLoader::CreateTextureFromFile(device, context, path, m_srv.GetAddressOf());
} // Init

ID3D11ShaderResourceView* Texture::GetSRV() const {
    return m_srv.Get();
} // GetSRV

float Texture::GetPixelHeight(int x, int y) const {
    if (m_cpuPixels.empty() || m_width == 0 || m_height == 0) {
        return 0.0f;
    }

    x = max(0, std::min(x, m_width - 1));
    y = max(0, std::min(y, m_height - 1));

    // R8G8B8A8 포맷이므로 픽셀당 4바이트씩 건너뜀
    // 흑백 높이맵이므로
    int i = ((y * m_width) + x) * 4;

    if (i >= m_cpuPixels.size()) return 0.0f;

    return static_cast<float>(m_cpuPixels[i]) / 255.0f;
} // GetPixelHeight

int Texture::GetWidth() const {
    return m_width;
} // GetWidth

int Texture::GetHeight() const {
    return m_height;
} // GetHeight

void Texture::SetFromGPU(ID3D11ShaderResourceView* srv, int w, int h, const std::vector<float>& cpuData) {
    m_srv = srv;
    m_width = w;
    m_height = h;
    m_cpuHeightPixels = cpuData;
} // SetFromGPU