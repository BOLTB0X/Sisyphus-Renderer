#include "Pch.h"
#include "NoiseGenerator.h"
#include "Resources/VolumeTexture.h"
// Utils
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
// define
#define CONSTANS_SLOT 0
#define VOLUME_SLOT   0
#define GROUP_SIZE    8

NoiseGenerator::NoiseGenerator() {
	m_noiseBuffer = nullptr;
    m_preNoiseBuffer.padding.x = -1.0f;
} // NoiseGenerator

NoiseGenerator::~NoiseGenerator() {
} // ~NoiseGenerator

bool NoiseGenerator::Init(ID3D11Device* device, HWND hwnd, const std::wstring& path) {
    using namespace ShaderHelper;

    if (!InitComputingShader(device, hwnd, path, m_computeShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<NoiseBuffer>(device, m_noiseBuffer.GetAddressOf())) {
        return false;
	}

    return true;
} // Init

void NoiseGenerator::Generate(ID3D11DeviceContext* context, VolumeTexture* target, const NoiseBuffer& data) {
    using namespace ShaderHelper;

    if (!target)
        return;

    if (memcmp(&m_preNoiseBuffer, &data, sizeof(NoiseBuffer)) == 0 
        || !UpdateConstantBuffer(context, m_noiseBuffer.Get(), data))
		return;

    context->CSSetShader(m_computeShader.Get(), nullptr, 0);
    context->CSSetConstantBuffers(CONSTANS_SLOT, 1, m_noiseBuffer.GetAddressOf());

    ID3D11UnorderedAccessView* uav = target->GetUAV();
    context->CSSetUnorderedAccessViews(VOLUME_SLOT, 1, &uav, nullptr);

    // 디스패치
    UINT groupX = static_cast<UINT>(data.textureSize.x) / GROUP_SIZE;
    UINT groupY = static_cast<UINT>(data.textureSize.y) / GROUP_SIZE;
    UINT groupZ = static_cast<UINT>(data.textureSize.z) / GROUP_SIZE;

    context->Dispatch(groupX, groupY, groupZ);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(VOLUME_SLOT, 1, &nullUAV, nullptr);
    context->CSSetShader(nullptr, nullptr, 0);
    m_preNoiseBuffer = data;
} // Generate
