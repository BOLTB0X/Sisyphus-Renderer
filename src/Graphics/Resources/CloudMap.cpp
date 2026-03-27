#include "Pch.h"
#include "CloudMap.h"
// Utils
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"
// define
#define CONSTANS_SLOT 0
#define MAP_SLOT      0
#define GRID_SIZE     8.0f
#define DISPATH_COUNT 1

using namespace ShaderHelper;
using namespace SharedConstants;

CloudMap::CloudMap() {
	m_preCloudBuffer.padding = -1.0f;
} // CloudMap

CloudMap::~CloudMap() {
} // ~CloudMap

bool CloudMap::Init(ID3D11Device* device, HWND hwnd, const std::wstring& path) {
    if (!InitComputingShader(device, hwnd, path, m_computeShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<CloudMapBuffer>(device, m_cloudBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // Init

void CloudMap::Generate(ID3D11DeviceContext* context, ID3D11UnorderedAccessView* target, const CloudMapBuffer& data) {
    if (!target)
        return;

    if (memcmp(&m_preCloudBuffer, &data, sizeof(CloudMapBuffer)) == 0
        || !UpdateConstantBuffer(context, m_cloudBuffer.Get(), data))
        return;

    context->CSSetShader(m_computeShader.Get(), nullptr, 0);
    context->CSSetConstantBuffers(CONSTANS_SLOT, 1, m_cloudBuffer.GetAddressOf());
    context->CSSetUnorderedAccessViews(MAP_SLOT, 1, &target, nullptr);

    UINT gridX = static_cast<UINT>(std::ceil(data.resolution.x / GRID_SIZE));
    UINT gridY = static_cast<UINT>(std::ceil(data.resolution.y / GRID_SIZE));
    context->Dispatch(gridX, gridY, DISPATH_COUNT);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(MAP_SLOT, 1, &nullUAV, nullptr);
} // Generate