#include "Pch.h"
#include "WeatherGenerator.h"
// Utils
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"
// define
#define CONSTANS_SLOT 3
#define MAP_SLOT      0

using namespace ShaderHelper;
using namespace SharedConstants;

WeatherGenerator::WeatherGenerator() 
    : m_gridSize(16.0f), m_dispatchCount(1){
} // CloudMap

WeatherGenerator::~WeatherGenerator() {
} // ~CloudMap

bool WeatherGenerator::Init(const InitParams& params) {
    if (!InitComputingShader(params.device, params.hwnd, params.path, m_computeShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<WeatherMapBuffer>(params.device, m_weatherBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // Init

void WeatherGenerator::Generate(ID3D11DeviceContext* context, const GenerateParams& params) {
    if (!params.target)
        return;

    if (!UpdateConstantBuffer(context, m_weatherBuffer.Get(), params.data)) {
        return;
    }

    context->CSSetShader(m_computeShader.Get(), nullptr, 0);
    context->CSSetConstantBuffers(CONSTANS_SLOT, 1, m_weatherBuffer.GetAddressOf());
    context->CSSetUnorderedAccessViews(MAP_SLOT, 1, &params.target, nullptr);

    UINT gridX = static_cast<UINT>(std::ceil(params.data.resolution.x / m_gridSize));
    UINT gridY = static_cast<UINT>(std::ceil(params.data.resolution.y / m_gridSize));
    context->Dispatch(gridX, gridY, m_dispatchCount);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(MAP_SLOT, 1, &nullUAV, nullptr);
} // Generate
