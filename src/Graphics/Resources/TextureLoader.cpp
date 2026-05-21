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

        // 계산을 쉽게 하기 위해 항상 R8G8B8A8 포맷으로 변환해서 뽑아냄
        DirectX::ScratchImage convertedImage;
        if (image.GetMetadata().format != DXGI_FORMAT_R8G8B8A8_UNORM) {
            hr = DirectX::Convert(
                image.GetImages(), image.GetImageCount(), image.GetMetadata(),
                DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT,
                DirectX::TEX_THRESHOLD_DEFAULT, convertedImage);
            if (SUCCEEDED(hr)) {
                const DirectX::Image* img = convertedImage.GetImage(0, 0, 0);
                outPixels->assign(img->pixels, img->pixels + img->slicePitch);
            }
        }
        else {
            const DirectX::Image* img = image.GetImage(0, 0, 0);
            outPixels->assign(img->pixels, img->pixels + img->slicePitch);
        } // if - else
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