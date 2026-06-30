#include "Pch.h"
#include "CloudMap.h"
#include "RenderTexture.h"
// Utils
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
// define
#define MAP_SLOT 0

using namespace ShaderHelper;
using namespace SharedConstants;
using namespace SharedConstants::PathConstants;

CloudMap::CloudMap() 
    : m_gridSize(8.0f), m_dispatchCount(1), m_resolution(1024) {
	m_resultRT = std::make_unique<RenderTexture>();
} // CloudMap

CloudMap::~CloudMap() {
} // ~CloudMap

bool CloudMap::Init(const InitParams& params) {
    if (!m_resultRT->Init(params.device, m_resolution, m_resolution,
        RenderTexture::RenderTextureType::UAV, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        return false;
    }

    if (!InitComputingShader(params.device, params.hwnd, CLOUD_MAP_CS, m_computeShader.GetAddressOf())) {
        return false;
    }

    return true;
} // Init

void CloudMap::Generate(ID3D11DeviceContext* context) {
    ID3D11UnorderedAccessView* pUAV = m_resultRT->GetUAV();

    context->CSSetShader(m_computeShader.Get(), nullptr, 0);
    context->CSSetUnorderedAccessViews(MAP_SLOT, 1, &pUAV, nullptr);

    UINT gridX = static_cast<UINT>(std::ceil(m_resolution / m_gridSize));
    UINT gridY = static_cast<UINT>(std::ceil(m_resolution / m_gridSize));
    context->Dispatch(gridX, gridY, m_dispatchCount);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(MAP_SLOT, 1, &nullUAV, nullptr);

    m_resultRT->GenerateMips(context);
} // GenerateVolume

ID3D11ShaderResourceView* CloudMap::GetSRV() {
    return m_resultRT->GetMippedSRV();
} // GetSRV