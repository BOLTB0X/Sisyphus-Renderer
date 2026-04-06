// DebugHelper.h
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>
#include <wrl/client.h>
#include <windows.h>
#include <string>
#include <DirectXMath.h>
#include <spdlog/spdlog.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

class VolumeTexture;

namespace DebugHelper { // HWND

    inline void ErrorBox(HWND hwnd, const std::wstring& message) {
        spdlog::error("Engine Error Box:");
        MessageBoxW(hwnd, message.c_str(), L"Engine Error", MB_OK | MB_ICONERROR);
    } // ErrorBox
} // DebugHelper

namespace DebugHelper { // 출력

    inline void DebugPrint(const std::string& message) {
        spdlog::debug(message);
    } // DebugPrint


    inline bool SuccessCheck(bool bSuccess, const std::string& msg) {
        if (bSuccess == true) return true;
        spdlog::error("{} (Status: Failed)", msg);
        return false;
    } // SuccessCheck
} // DebugHelper

namespace DebugHelper {
    class VolumeSlicer {
    public:
        VolumeSlicer();
        ~VolumeSlicer();

        bool  Init(ID3D11Device*, HWND, ID3D11SamplerState*);
        void  Update(ID3D11DeviceContext*, VolumeTexture*, float);
        float GetDepth() const;
        void  OnGui();

    private:
        struct SlicerParams {
            float sSliceDepth;
            DirectX::XMFLOAT3 sPadding;
        };

        // 셰이더 리소스
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_slicerComputeShader;
        Microsoft::WRL::ComPtr<ID3D11Buffer>        m_slicerBuffer;
        ID3D11SamplerState*                         m_linearSampler;

        // 3축 단면용 2D 리소스 (0: XY, 1: YZ, 2: XZ)
        Microsoft::WRL::ComPtr<ID3D11Texture2D>            m_debugSlices[3];
        Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>  m_debugUAVs[3];
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>   m_debugSRVs[3];

        float m_currentDepth;
        UINT  m_sliceSize;
    }; // VolumeSlicer
}