#include "Pch.h"
#include "SkyBox.h"
// Resources
#include "Resources/DefaultMesh.h"
#include "Resources/VolumeTexture.h"
// Data
#include "Data/RenderTexture.h"
// D3D11
#include "D3D11/D3D11State.h"
// Utils
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/ScreenConstants.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
// define
#define WEATHER_SIZE           1024
#define BUFFER_SLOT_WORLD      2
#define BUFFER_SLOT_CHECK      3
#define BUFFER_SLOT_ATMOSPHERE 3
#define BUFFER_SLOT_CLOUDBOX   4
#define SAMPLER_SLOT           0
#define TEX_SLOT_DEPTH         1
#define TEX_SLOT_LUT           2

using namespace DirectX;
using namespace SharedConstants;
using namespace PathConstants;
using namespace ConstantBuffer;

SkyBox::SkyBox() {
	m_CubeMesh = std::make_unique<DefaultMesh>();
    m_linerWrapSampler = nullptr;
    m_depthSRV = nullptr;
} // SkyBox

SkyBox::~SkyBox() {
    m_linerWrapSampler = nullptr;
    m_depthSRV = nullptr;
} // ~SkyBox

bool SkyBox::Init(const InitParams& params) {
    if (!params.device || !params.context) {
        return false;
    }

    if (!m_CubeMesh->Init(params.device, 1, DefaultMesh::DefaultMeshType::Cube)) {
        return false;
    }


    if (!InitShader(params.device, params.hwnd)) {
        return false;
    }

    m_linerWrapSampler = params.sampler;
    m_depthSRV = params.depth;
    return true;
} // Init

bool SkyBox::InitShader(ID3D11Device* device, HWND hwnd) {
    using namespace ShaderHelper;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (!InitVertexShader(device, hwnd, PathConstants::SKYBOX_VS,
        layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(device, hwnd, PathConstants::SKYBOX_PS, m_pixelShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<WorldBuffer>(device, m_worldBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<CheckBuffer>(device, m_checkBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // InitShader

void SkyBox::Render(ID3D11DeviceContext* context, const RenderParams& params) {
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    XMMATRIX world = XMMatrixScaling(500.0f, 500.0f, 500.0f);
    m_WorldData.world = XMMatrixTranspose(world);
    if (!ShaderHelper::UpdateConstantBuffer(context, m_worldBuffer.Get(), m_WorldData)) {
        return;
    }
   context->VSSetConstantBuffers(BUFFER_SLOT_WORLD, 1, m_worldBuffer.GetAddressOf());

    m_isReflection.flag = params.isReflection;
    if (!ShaderHelper::UpdateConstantBuffer(context, m_checkBuffer.Get(), m_isReflection)) {
        return;
    }

    context->PSSetConstantBuffers(BUFFER_SLOT_CHECK, 1, m_checkBuffer.GetAddressOf());
    context->PSSetSamplers(SAMPLER_SLOT, 1, &m_linerWrapSampler);
    context->PSSetShaderResources(TEX_SLOT_DEPTH, 1, &m_depthSRV);
    context->PSSetShaderResources(TEX_SLOT_LUT, 1, &params.skyLUT);

    m_CubeMesh->RenderBuffer(context);
    context->DrawIndexed(m_CubeMesh->GetIndexCount(), 0, 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(TEX_SLOT_DEPTH, 1, &nullSRV);
	context->PSSetShaderResources(TEX_SLOT_LUT, 1, &nullSRV);

    if (m_isReflection.flag) {
        m_isReflection.flag = 0;
    }
} // Render

void SkyBox::OnGui() {
    ImGui::Begin("Sky & AtmosphereLUT Control");

    ImGui::Separator();
    
    ImGui::End();
} // OnGui
