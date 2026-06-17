#include "Pch.h"
#include "SceneRTManager.h"
#include "RendererState.h"
// Utils
#include "Helpers/DebugHelper.h"
#include "SharedConstants/PathConstants.h"

using namespace DebugHelper;
using namespace SharedConstants;

SceneRTManager::SceneRTManager() {
	m_SceneRTs = std::unordered_map<std::string, std::shared_ptr<RenderTexture>>();
} // SceneRTManager

SceneRTManager::~SceneRTManager() {
	m_SceneRTs.clear();
} // ~SceneRTManager

bool SceneRTManager::Init(ID3D11Device* device, int width, int height) {
    bool result = true;

    result &= CreateRT(device, PathConstants::KEY_SCENE_RT, width, height);
    result &= CreateRT(device, PathConstants::KEY_REFLECTION_RT, width, height);
    result &= CreateRT(device, PathConstants::KEY_REFRACTION_RT, width, height);

    if (result == false) {
        DebugHelper::DebugPrint("RTManager: 기본 렌더 타깃 초기화 실패!");
    }

    return result;
} // Init

bool SceneRTManager::CreateRT(ID3D11Device* device, const std::string& name, int width, int height,
    RenderTexture::RenderTextureType type, DXGI_FORMAT format) {
    if (m_SceneRTs.find(name) != m_SceneRTs.end()) {
        return true;
    }

    auto rt = std::make_shared<RenderTexture>();
    if (!rt->Init(device, width, height, type, format)) {
        return false;
    }

    m_SceneRTs[name] = std::move(rt);
    return true;
} // CreateRT

std::shared_ptr<RenderTexture> SceneRTManager::GetRT(const std::string& name) {
    auto it = m_SceneRTs.find(name);
    if (it != m_SceneRTs.end()) {
        return it->second;
    }

    DebugHelper::DebugPrint("RTManager, 등록되지 않은 렌더 타깃 요청 - " + name);
    return nullptr;
} // GetRT