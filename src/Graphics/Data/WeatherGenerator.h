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
        DirectX::XMFLOAT2 padding1;
		// [Row 2]
        DirectX::XMFLOAT3 seed;
		float             padding2;

        WeatherMapBuffer() {
            resolution = SharedConstants::BuffersConstants::WEATHER_MAP_RESOLUTION;
            padding1 = DirectX::XMFLOAT2(0.0f, 0.0f);
			seed = DirectX::XMFLOAT3(13.0f, 17.0f, 19.0f);
			padding2 = 0.0f;
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