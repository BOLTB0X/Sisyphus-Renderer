#include "Pch.h"
#include "NoiseGenerator.h"
#include "Resources/VolumeTexture.h"
// Utils
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"

using namespace ConstantBuffer;

NoiseGenerator::NoiseGenerator() {
	m_noiseBuffer = nullptr;
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
    context->CSSetConstantBuffers(0, 1, m_noiseBuffer.GetAddressOf());

    ID3D11UnorderedAccessView* uav = target->GetUAV();
    context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

    // 디스패치
    // 스레드 그룹 크기가 (8, 8, 8)이므로 해상도를 8로 나눈 만큼 그룹을 보내야함
    // ex: 128 / 8 = 16그룹
    UINT groupX = static_cast<UINT>(data.textureSize.x) / 8;
    UINT groupY = static_cast<UINT>(data.textureSize.y) / 8;
    UINT groupZ = static_cast<UINT>(data.textureSize.z) / 8;

    context->Dispatch(groupX, groupY, groupZ);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
    context->CSSetShader(nullptr, nullptr, 0);
    m_preNoiseBuffer = data;
} // Generate
