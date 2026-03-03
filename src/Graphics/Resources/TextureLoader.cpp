#include "Pch.h"
#include "TextureLoader.h"
#include "Helpers/DebugHelper.h"
// STL
#include <filesystem>
#include <algorithm>
// DirectXTK
#include <DirectXTex.h>

bool TextureLoader::CreateTextureFromFile(
    ID3D11Device* device,
    ID3D11DeviceContext* context,
    const std::string& filename,
    ID3D11ShaderResourceView** outSRV) {
    HRESULT hr = S_OK;
    DirectX::ScratchImage image;

    std::wstring wFilename(filename.begin(), filename.end());
    std::string ext = std::filesystem::path(filename).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".dds") {
        hr = DirectX::LoadFromDDSFile(wFilename.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
    } else if (ext == ".tga") {
        hr = DirectX::LoadFromTGAFile(wFilename.c_str(), nullptr, image);
    } else {
        hr = DirectX::LoadFromWICFile(wFilename.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);
    }

    if (FAILED(hr)) {
        DebugHelper::DebugPrint(filename + " 로드 실패");
        return false;
    }

    if (image.GetMetadata().mipLevels == 1) {
        DirectX::ScratchImage mipChain;
        hr = DirectX::GenerateMipMaps(
            image.GetImages(), image.GetImageCount(), image.GetMetadata(),
            DirectX::TEX_FILTER_DEFAULT, 0, mipChain);

        if (SUCCEEDED(hr)) {
            image = std::move(mipChain);
        }
    }

    hr = DirectX::CreateShaderResourceView(
        device,
        image.GetImages(),
        image.GetImageCount(),
        image.GetMetadata(),
        outSRV
    );

    return SUCCEEDED(hr);
} // CreateTextureFromFile