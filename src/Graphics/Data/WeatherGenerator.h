#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <directxmath.h>
#include "Utils/SharedConstants/BuffersConstants.h"

class WeatherGenerator {
public:
    struct InitParams {
        ID3D11Device* device;
        HWND         hwnd;
        std::wstring path;

        InitParams() : device(nullptr), hwnd(nullptr), path(L"") {
        }
	}; // InitParams

    struct WeatherMapBuffer {
        // [Row 1]
        DirectX::XMFLOAT2 resolution;
        float  time;
        float  coverage; // 구름 분포도 (0.0: 맑음 ~ 1.0: 흐림)

        WeatherMapBuffer() {
            resolution = SharedConstants::BuffersConstants::RESOLUTION;
            time = 0.0f;
            coverage = SharedConstants::BuffersConstants::COVERAGE;
        }
    }; // WeatherMapBuffer

    struct GenerateParams {
		ID3D11UnorderedAccessView* target;
        WeatherMapBuffer            data;

        GenerateParams() : target(nullptr), data() {
		}
	}; // RenderParams

public:
    WeatherGenerator();
	~WeatherGenerator();

    bool Init(const InitParams&);
    void Generate(ID3D11DeviceContext*, const GenerateParams&);
	void OnGui(); // Imgui 용

private:
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_weatherBuffer;
    WeatherMapBuffer                            m_weatherData;
    float                                       m_gridSize;
    UINT                                        m_dispatchCount;
}; // WeatherGenerator