#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <directxmath.h>
#include "Utils/SharedConstants/BuffersConstants.h"

class CloudMap {
public:
	struct CloudMapBuffer {
        // [Row 1]
		DirectX::XMFLOAT2 resolution;
        float  time;
        float  coverage; // 구름 분포도 (0.0: 맑음 ~ 1.0: 흐림)
        // [Row 2]
        float  perlinFbmFreq; // 8
        float  perlinOctaves; // 4
        float  perlinBias; // 0.25
        float  worleyFbmFreq; // 8
        // [Row 3]
        DirectX::XMFLOAT2 windDirection; // 바람이 부는 2D 방향 (예: 1.0, 0.0 이면 X축 이동)
        float  windSpeed; // 바람의 세기
        float  padding;

        CloudMapBuffer() {
            resolution = SharedConstants::BuffersConstants::RESOLUTION;
            time = 0.0f;
            coverage = SharedConstants::BuffersConstants::COVERAGE;
            perlinFbmFreq = SharedConstants::BuffersConstants::PERLIN_FBM_FREQ;
            perlinOctaves = SharedConstants::BuffersConstants::PERLIN_OCTAVES;
            perlinBias = SharedConstants::BuffersConstants::PERLIN_BIAS;
            worleyFbmFreq = SharedConstants::BuffersConstants::WORLEY_FBM_FREQ;
            windDirection = SharedConstants::BuffersConstants::WIND_DIRECTION;
            windSpeed = SharedConstants::BuffersConstants::WIND_SPEED;
            padding = 0.0f;
        }

        CloudMapBuffer(float t) {
            resolution = SharedConstants::BuffersConstants::RESOLUTION;
            time = t;
            coverage = SharedConstants::BuffersConstants::COVERAGE;
            perlinFbmFreq = SharedConstants::BuffersConstants::PERLIN_FBM_FREQ;
            perlinOctaves = SharedConstants::BuffersConstants::PERLIN_OCTAVES;
            perlinBias = SharedConstants::BuffersConstants::PERLIN_BIAS;
            worleyFbmFreq = SharedConstants::BuffersConstants::WORLEY_FBM_FREQ;
            windDirection = SharedConstants::BuffersConstants::WIND_DIRECTION;
            windSpeed = SharedConstants::BuffersConstants::WIND_SPEED;
            padding = 0.0f;
        }
	}; // CloudMapBuffer

public:
	CloudMap();
	~CloudMap();

    bool Init(ID3D11Device*, HWND, const std::wstring& path);
    void Generate(ID3D11DeviceContext*, ID3D11UnorderedAccessView*, const CloudMapBuffer&);

private:
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_cloudBuffer;
    CloudMapBuffer                              m_preCloudBuffer;
}; // CloudMap