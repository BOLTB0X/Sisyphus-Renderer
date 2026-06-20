#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <unordered_map>
#include <string>
#include <memory>
// Data
#include "Data/RenderTexture.h"

class SceneRTManager {
public:
    SceneRTManager();
    SceneRTManager(const SceneRTManager&) = delete;
    ~SceneRTManager();

    bool Init(ID3D11Device*, int, int);

    bool CreateRT(ID3D11Device*, const std::string&, int, int,
        RenderTexture::RenderTextureType type = RenderTexture::RenderTextureType::Normal,
        DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT);

    std::shared_ptr<RenderTexture> GetRT(const std::string&);

private:
    std::unordered_map<std::string, std::shared_ptr<RenderTexture>> m_SceneRTs;
}; // SceneRTManager