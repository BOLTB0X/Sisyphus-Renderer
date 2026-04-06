#include "Pch.h"
#include "DebugHelper.h"
#include "Resources/VolumeTexture.h"
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

using namespace SharedConstants;

namespace DebugHelper {

    VolumeSlicer::VolumeSlicer() {
        m_linearSampler = nullptr;
        m_currentDepth = 0.5f;
        m_sliceSize = 256;
    } // VolumeSlicer

    VolumeSlicer::~VolumeSlicer() {
        m_linearSampler = nullptr;
    } // ~VolumeSlicer

    bool VolumeSlicer::Init(ID3D11Device* device, HWND hwnd, ID3D11SamplerState* linerSampler) {
        using namespace ShaderHelper;

        if (!InitComputingShader(device, hwnd, PathConstants::SLICER_CS, m_slicerComputeShader.GetAddressOf())) {
            return SuccessCheck(false, "Slicer CS Init");
        }

        if (!InitConstantBuffer<SlicerParams>(device, m_slicerBuffer.GetAddressOf())) {
            return SuccessCheck(false, "Slicer 상수버퍼 Init");
        }

        m_linearSampler = linerSampler;

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = m_sliceSize;
        texDesc.Height = m_sliceSize;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

        for (int i = 0; i < 3; ++i) {
            if (FAILED(device->CreateTexture2D(&texDesc, nullptr, m_debugSlices[i].GetAddressOf()))) return false;
            if (FAILED(device->CreateUnorderedAccessView(m_debugSlices[i].Get(), nullptr, m_debugUAVs[i].GetAddressOf()))) return false;
            if (FAILED(device->CreateShaderResourceView(m_debugSlices[i].Get(), nullptr, m_debugSRVs[i].GetAddressOf()))) return false;
        }

        return SuccessCheck(true, "VolumeSlicer Successfully Initialized");
    } // Init

    void VolumeSlicer::Update(ID3D11DeviceContext* context, VolumeTexture* target, float depth) {
        if (!target || !m_slicerComputeShader) return;

        m_currentDepth = depth;

        // 상수 버퍼 업데이트
        SlicerParams params = { depth, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) };
        ShaderHelper::UpdateConstantBuffer(context, m_slicerBuffer.Get(), params);

        // 리소스 바인딩 및 실행
        context->CSSetShader(m_slicerComputeShader.Get(), nullptr, 0);
        context->CSSetConstantBuffers(0, 1, m_slicerBuffer.GetAddressOf());

        ID3D11ShaderResourceView* targetSRV = target->GetSRV();
        context->CSSetShaderResources(0, 1, &targetSRV);
        context->CSSetSamplers(0, 1, &m_linearSampler);

        context->CSSetUnorderedAccessViews(0, 1, m_debugUAVs[0].GetAddressOf(), nullptr);
        context->CSSetUnorderedAccessViews(1, 1, m_debugUAVs[1].GetAddressOf(), nullptr);
        context->CSSetUnorderedAccessViews(2, 1, m_debugUAVs[2].GetAddressOf(), nullptr);

        context->Dispatch(m_sliceSize / 8, m_sliceSize / 8, 1);

        // UAV 해제
        ID3D11UnorderedAccessView* nullUAV = nullptr;
        context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
        context->CSSetUnorderedAccessViews(1, 1, &nullUAV, nullptr);
        context->CSSetUnorderedAccessViews(2, 1, &nullUAV, nullptr);

        ID3D11ShaderResourceView* nullSRV = nullptr;
        context->CSSetShaderResources(0, 1, &nullSRV);
    } // Update

    float VolumeSlicer::GetDepth() const {
        return m_currentDepth;
    } // GetDepth

    void VolumeSlicer::OnGui() {
        if (ImGui::Begin("Volume Slicer Debug")) {
            ImGui::SliderFloat("Slice Depth", &m_currentDepth, 0.0f, 1.0f);
            ImGui::Separator();
            const char* titles[] = { "XY (Front)", "YZ (Side)", "XZ (Top)" };
            for (int i = 0; i < 3; ++i) {
                ImGui::BeginGroup();
                if (m_debugSRVs[i]) {
                    ImGui::Text("%s", titles[i]);
                    ImGui::Image((ImTextureID)m_debugSRVs[i].Get(), ImVec2(200.0f, 200.0f));
                }
                ImGui::EndGroup();
                if (i < 2) ImGui::SameLine();
            }
        }
        ImGui::End();
    }
} // VolumeSlicer