#include "Pch.h"
#include "VolumetricCloud.h"
#include "D3D11/D3D11State.h"
#include "RenderTexture.h"
//Utils
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"

using namespace SharedConstants;
using namespace ShaderHelper;
using namespace DirectX;

VolumetricCloud::VolumetricCloud() {
	m_resultRT = std::make_unique<RenderTexture>();
	m_linerWrapSampler = nullptr;
	m_WeatherMapSRV = nullptr;
	m_baseNoiseSRV = nullptr;
	m_detailNoiseSRV = nullptr;
	m_blueNoiseSRV = nullptr;
	m_cloudBufferData = {};
	m_prevCloudBufferData = {};
	m_prevResolutionData.padding.x = -1.0f;
} // VolumetricCloud

VolumetricCloud::~VolumetricCloud() {
	m_linerWrapSampler = nullptr;
	m_WeatherMapSRV = nullptr;
	m_baseNoiseSRV = nullptr;
	m_detailNoiseSRV = nullptr;
	m_blueNoiseSRV = nullptr;
} // ~VolumetricCloud

bool VolumetricCloud::Init(const InitParams& params) {
	UINT halfWidth = SharedConstants::ScreenConstants::WIDTH / 2;
	UINT halfHeight = SharedConstants::ScreenConstants::HEIGHT / 2;

	m_linerWrapSampler = params.sampler;
	m_WeatherMapSRV = params.weatherMapSRV;
	m_baseNoiseSRV = params.baseNoiseSRV;
    m_detailNoiseSRV = params.detailNoiseSRV;
    m_blueNoiseSRV = params.blueNoiseSRV;

    if (!m_resultRT->Init(params.device, halfWidth, halfHeight,
        RenderTexture::RenderTextureType::UAV, DXGI_FORMAT_R16G16B16A16_FLOAT)) {
        return false;
    }

    if (!InitComputingShader(params.device, params.hwnd, 
		PathConstants::VOLUMETRIC_CLOUD_CS, m_computeShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<CloudBuffer>(params.device, m_cloudBuffer.GetAddressOf()) ||
        !InitConstantBuffer<ResolutionBuffer>(params.device, m_resolutionBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // Init