#include "Pch.h"
#include "NoiseGenerator.h"
#include "Resources/VolumeTexture.h"
// Utils
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
// define
#define CONSTANS_SLOT 3
#define VOLUME_SLOT   0

NoiseGenerator::NoiseGenerator() {
    m_grupeSize = 0;
} // NoiseGenerator

NoiseGenerator::~NoiseGenerator() {
} // ~NoiseGenerator

bool NoiseGenerator::Init(const InitParams& params) {
    using namespace ShaderHelper;

    m_grupeSize = params.groupSize;

    if (!InitComputingShader(params.device, params.hwnd, params.path, m_computeShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<NoiseBuffer>(params.device, m_noiseBuffer.GetAddressOf())) {
        return false;
	}

    return true;
} // Init

void NoiseGenerator::Generate(ID3D11DeviceContext* context, const GenerateParams& params) {
    using namespace ShaderHelper;

    if (!params.target)
        return;

    if (!UpdateConstantBuffer(context, m_noiseBuffer.Get(), params.data))
        return;

    context->CSSetShader(m_computeShader.Get(), nullptr, 0);
    context->CSSetConstantBuffers(CONSTANS_SLOT, 1, m_noiseBuffer.GetAddressOf());

    ID3D11UnorderedAccessView* uav = params.target->GetUAV();
    context->CSSetUnorderedAccessViews(VOLUME_SLOT, 1, &uav, nullptr);

    // 디스패치
    UINT groupX = static_cast<UINT>(params.data.textureSize.x) / m_grupeSize;
    UINT groupY = static_cast<UINT>(params.data.textureSize.y) / m_grupeSize;
    UINT groupZ = static_cast<UINT>(params.data.textureSize.z) / m_grupeSize;

    context->Dispatch(groupX, groupY, groupZ);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(VOLUME_SLOT, 1, &nullUAV, nullptr);
    context->GenerateMips(params.target->GetSRV());
    context->CSSetShader(nullptr, nullptr, 0);
} // Generate
