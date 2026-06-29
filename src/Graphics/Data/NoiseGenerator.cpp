#include "Pch.h"
#include "NoiseGenerator.h"
#include "Resources/VolumeTexture.h"
#include "Resources/Texture.h"
// Utils
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
// define
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

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = 1024; // 예시 해상도
    desc.Height = 1024;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R32_FLOAT;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

    params.device->CreateTexture2D(&desc, nullptr, m_gpuTex.GetAddressOf());
    params.device->CreateShaderResourceView(m_gpuTex.Get(), nullptr, m_srv.GetAddressOf());
    params.device->CreateUnorderedAccessView(m_gpuTex.Get(), nullptr, m_uav.GetAddressOf());

    // Staging 텍스처 생성
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    params.device->CreateTexture2D(&desc, nullptr, m_stagingTex.GetAddressOf());

    return true;
} // Init

void NoiseGenerator::Generate2D(ID3D11DeviceContext* context, const Generate2DParams& params) {
    using namespace ShaderHelper;

    context->CSSetShader(m_computeShader.Get(), nullptr, 0);
    ID3D11UnorderedAccessView* uavArr[] = { m_uav.Get() };
    context->CSSetUnorderedAccessViews(0, 1, uavArr, nullptr);

    UINT groupX = params.width / m_grupeSize;
    UINT groupY = params.height / m_grupeSize;
    context->Dispatch(groupX, groupY, 1);

    // UAV 해제
    ID3D11UnorderedAccessView* nullUAV[] = { nullptr };
    context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
    context->CSSetShader(nullptr, nullptr, 0);

    // Staging 텍스처로 데이터 복사
    context->CopyResource(m_stagingTex.Get(), m_gpuTex.Get());

    // CPU 메모리로 데이터 Map (Readback)
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(context->Map(m_stagingTex.Get(), 0, D3D11_MAP_READ, 0, &mapped))) {
        std::vector<float> cpuData(params.width * params.height);

        // 데이터 행 단위 복사 (RowPitch 고려)
        const uint8_t* srcRow = static_cast<const uint8_t*>(mapped.pData);
        for (UINT y = 0; y < params.height; ++y) {
            memcpy(&cpuData[y * params.width], srcRow, params.width * sizeof(float));
            srcRow += mapped.RowPitch; // 다음 행으로 이동
        }

        context->Unmap(m_stagingTex.Get(), 0);
        params.outputTexture->SetFromGPU(m_srv.Get(), params.width, params.height, cpuData);
    }
} // Generate2D

void NoiseGenerator::GenerateVolume(ID3D11DeviceContext* context, const Generate3DParams& params) {
    using namespace ShaderHelper;

    if (!params.target) {
        return;
    }

    context->CSSetShader(m_computeShader.Get(), nullptr, 0);

    ID3D11UnorderedAccessView* uav = params.target->GetUAV();
    context->CSSetUnorderedAccessViews(VOLUME_SLOT, 1, &uav, nullptr);

    // 디스패치
    UINT groupX = static_cast<UINT>(params.resolution.x) / m_grupeSize;
    UINT groupY = static_cast<UINT>(params.resolution.y) / m_grupeSize;
    UINT groupZ = static_cast<UINT>(params.resolution.z) / m_grupeSize;

    context->Dispatch(groupX, groupY, groupZ);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(VOLUME_SLOT, 1, &nullUAV, nullptr);
    context->GenerateMips(params.target->GetSRV());
    context->CSSetShader(nullptr, nullptr, 0);
} // GenerateVolume
