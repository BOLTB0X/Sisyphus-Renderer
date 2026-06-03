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
    ID3D11ShaderResourceView** outSRV,
    std::vector<unsigned char>* outPixels,
    int* outWidth,
    int* outHeight) {
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

    if (outPixels && outWidth && outHeight) {
        *outWidth = static_cast<int>(image.GetMetadata().width);
        *outHeight = static_cast<int>(image.GetMetadata().height);

        const DirectX::Image* img = nullptr;
        DirectX::ScratchImage convertedImage;

        if (image.GetMetadata().format != DXGI_FORMAT_R8G8B8A8_UNORM) {
            hr = DirectX::Convert(
                image.GetImages(), image.GetImageCount(), image.GetMetadata(),
                DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT,
                DirectX::TEX_THRESHOLD_DEFAULT, convertedImage);

            if (SUCCEEDED(hr)) {
                img = convertedImage.GetImage(0, 0, 0);
            }
        }
        else {
            img = image.GetImage(0, 0, 0);
        }

        if (img) {
            size_t pureRowBytes = (*outWidth) * 4;
            outPixels->resize((*outHeight) * pureRowBytes);

            for (int y = 0; y < *outHeight; ++y) {
                uint8_t* dest = outPixels->data() + (y * pureRowBytes);
                const uint8_t* src = img->pixels + (y * img->rowPitch);
                memcpy(dest, src, pureRowBytes);
            }
        }
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